#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

std::string read_wdf_file (std::ifstream& file_stream)
{
    file_stream.seekg (0, std::ios::end);
    const auto file_size_bytes = file_stream.tellg();
    std::string buffer (file_size_bytes, '\0');
    file_stream.seekg (0);
    file_stream.read (buffer.data(), file_size_bytes);
    return buffer;
}

struct File_Sections
{
    std::string_view circuit {};
    std::string_view inputs {};
    std::string_view outputs {};
};

template <typename T, int max_n>
struct Array
{
    T data[max_n] {};
    int count {};
};

File_Sections parse_file_sections (std::string_view file_str)
{
    File_Sections sections {};

    enum Section_Type
    {
        Circuit,
        Inputs,
        Outputs,
    };
    struct Section_Info
    {
        size_t start_in_file {};
        size_t start_after_tag {};
        Section_Type type {};
    };

    Array<Section_Info, 3> section_info {};
    const auto add_section_info = [&section_info, &file_str] (std::string_view tag, Section_Type type)
    {
        const auto section_start = file_str.find (tag);
        if (section_start == std::string_view::npos)
            return;

        const auto start_after_tag = section_start + tag.size();
        section_info.data[section_info.count++] = { section_start, start_after_tag, type };
    };

    add_section_info ("circuit:", Circuit);
    add_section_info ("inputs:", Inputs);
    add_section_info ("outputs:", Outputs);

    std::sort (section_info.data,
               section_info.data + section_info.count,
               [] (const Section_Info& a, const Section_Info& b)
               {
                   return a.start_in_file > b.start_in_file;
               });

    for (int i = 0; i < section_info.count; ++i)
    {
        const auto& info = section_info.data[i];
        const auto section_str = file_str.substr (info.start_after_tag);
        file_str = file_str.substr (0, info.start_in_file);

        if (info.type == Circuit)
            sections.circuit = section_str;
        else if (info.type == Inputs)
            sections.inputs = section_str;
        else if (info.type == Outputs)
            sections.outputs = section_str;
    }

    return sections;
}

int main (int argc, char* argv[])
{
    std::cout << "Compiling WDF from file: " << argv[1] << '\n';

    std::ifstream wdf_file { argv[1] };
    if (! wdf_file.is_open()) {
        std::cout << "Unable to open WDF file!\n";
        return 1;
    }

    const auto file_str = read_wdf_file (wdf_file);

    const auto sections = parse_file_sections (file_str);

    std::cout << "circuit:";
    std::cout << sections.circuit;
    std::cout << "inputs:";
    std::cout << sections.inputs;
    std::cout << "outputs:";
    std::cout << sections.outputs;

    return 0;
}
