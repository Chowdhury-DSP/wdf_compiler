import glob
import subprocess
import pathlib

path = pathlib.Path(__file__).parent.resolve()
compiler = f"../../compiler/wdf_compiler"

for test_file in glob.glob(f'{path}/*'):
    test_file_path = pathlib.Path(test_file)
    print(f"Testing file: {test_file_path.name}")

    proc = subprocess.Popen([f"{compiler}", f"{test_file_path.name}", "out.h"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    proc.wait()
    output = proc.stdout.read().decode()
    output_err = proc.stderr.read().decode()

    print(proc.returncode)
    print(output)
    print(output_err)
    assert("Panic" not in output)
    assert("Panic" not in output_err)
    assert(proc.returncode == 1)
