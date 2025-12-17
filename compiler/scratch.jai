SCRATCH_SIZE :: 1 << 13;
scratch: Temporary_Storage;
reset_scratch :: () {
    scratch.data = scratch.original_data;
    scratch.size = scratch.original_size;
    scratch.current_page_bytes_occupied = 0;
    scratch.total_bytes_occupied = 0;
    scratch.high_water_mark = 0;
}
start_scratch :: () #expand {
    `scratch_state: Temporary_Storage_State = ---;
    scratch_state.current_page_bytes_occupied = scratch.current_page_bytes_occupied;
    scratch_state.total_bytes_occupied        = scratch.total_bytes_occupied;
    scratch_state.top_overflow_page           = scratch.overflow_pages;
    context.temporary_storage = *scratch;
    defer {
        if scratch_state.top_overflow_page {
            page := scratch_state.top_overflow_page;
            scratch.data = (cast(*u8) page) + ALIGNED_OVERFLOW_PAGE_ALLOCATION;
            scratch.size = page.size;
            assert(page.size >= 0);
        } else {
            scratch.data = scratch.original_data;
            scratch.size = scratch.original_size;
            assert(scratch.overflow_pages == null);
        }

        scratch.current_page_bytes_occupied = scratch_state.current_page_bytes_occupied;
        scratch.total_bytes_occupied        = scratch_state.total_bytes_occupied;
    }
}

ts_allocator_proc :: (mode: Allocator_Mode, requested_size: s64, old_size: s64, old_memory: *void, allocator_data: *void) -> *void {
    if #complete mode == {
      case .FREE;
        return null;
      case .RESIZE;
        real_old_size := (old_size + 7) & ~7; // We know old_size was padded
        if requested_size <= real_old_size return old_memory;
        #through;
      case .ALLOCATE;
        // Pad to 8-byte alignment.
        size := (requested_size + 7) & ~7;

        ts := cast(*Temporary_Storage)allocator_data;

        // If someone gave us a context with no temporary_storage, fall back to context.allocator.
        if !ts {
            a := context.allocator;
            if a.proc == temporary_allocator_proc {
                // context.allocator is temporary_allocator, so, fall back to default_allocator.
                a.proc = Context_Base.default_allocator.proc;
                a.data = null;
            }

            return a.proc(mode, requested_size, old_size, old_memory, a.data);
        }

        if size > ts.size - ts.current_page_bytes_occupied {
            success := add_new_page(ts, size);
            if !success  return null;
        }

        result := ts.data + ts.current_page_bytes_occupied;
        if result && (mode == .RESIZE) {
            memcpy(result, old_memory, min(old_size, requested_size));
        }

        ts.current_page_bytes_occupied += cast(s32) size;
        ts.total_bytes_occupied += cast(s32) size;
        ts.high_water_mark = max(ts.high_water_mark, ts.total_bytes_occupied);

        return result;

      case .STARTUP;      #through;
      case .SHUTDOWN;
        return null;

      case .THREAD_START; #through;
      case .THREAD_STOP;
        assert(false, "Multithreaded access is not supported by temporary_allocator_proc.\n");
        return null;

      case .CREATE_HEAP; #through;
      case .DESTROY_HEAP;
        assert(false, "Create/Destroy heap are not supported by temporary_allocator_proc.\n");
        return null;

      case .IS_THIS_YOURS;
        ts := context.temporary_storage;
        if !ts return null;

        if (old_memory >= ts.data) && (old_memory < ts.data + ts.size) return cast(*void) true;
        if (old_memory >= ts.original_data) && (old_memory < ts.original_data + ts.original_size) return cast(*void) true;

        page := ts.overflow_pages;
        while page {
            if (old_memory >= page) && (old_memory < cast(*void)page + page.size + ALIGNED_OVERFLOW_PAGE_ALLOCATION) return cast(*void) true;
            page = page.next;
        }

        // Not found!
        return null;

      case .CAPS;
        if old_memory { (cast(*string)old_memory).* = "modules/Basic temporary_allocator_proc"; }
        return cast(*void)(Allocator_Caps.HINT_I_AM_PER_FRAME_TEMPORARY_STORAGE);
    }
}

ALIGNED_OVERFLOW_PAGE_ALLOCATION :: 32;
add_new_page :: (ts: *Temporary_Storage, minimum_size: s64) -> bool {
    #assert(size_of(Temporary_Storage.Overflow_Page) <= ALIGNED_OVERFLOW_PAGE_ALLOCATION);

    data_size : s64 = ts.original_size - ALIGNED_OVERFLOW_PAGE_ALLOCATION;
    if data_size < minimum_size  data_size = minimum_size;

    size_including_header := data_size + ALIGNED_OVERFLOW_PAGE_ALLOCATION;

    // Pad to 8-byte alignment.
    size_including_header = (size_including_header + 7) & ~7;

    // It's legal for overflow_allocator to be null, in which case
    // we do whatever alloc does.
    allocator      := ts.overflow_allocator;
    if !allocator.proc {
        // We don't know what allocator might be pushed onto the context,
        // so, fall back to __default_allocator.
        allocator      = Context_Base.default_allocator;
    }

    memory := alloc(size_including_header,, allocator=allocator);
    if !memory {
        log_error("Failed to allocate % bytes for a Temporary_Storage overflow page.\n", size_including_header);
        return false;  // This is going to be bad!
    }

    page := cast(*ts.Overflow_Page) memory;
    page.next           = ts.overflow_pages;
    page.allocator      = allocator;
    page.size           = data_size;

    ts.overflow_pages = page;

    ts.data = memory + ALIGNED_OVERFLOW_PAGE_ALLOCATION;
    ts.size = data_size;
    ts.current_page_bytes_occupied = 0;
    assert(ts.size >= 0);

    return true;
}
