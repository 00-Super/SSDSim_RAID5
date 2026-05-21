import subprocess

def run_c_program(program_path, input_file=None):
    command = [program_path]+input_file
    print(command)

    try:
        #print("ssd:"+program_path+" trace: "+input_file[1])
        process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,bufsize=1, universal_newlines=True)
        while True:
             process.stdin.flush()
             output = process.stdout.readline()
             if output == '' and process.poll() is not None:
                break
             print(output,end="",flush=True)
        if result.returncode == 0:
            print("程序成功执行")
        else:
            print(f"程序执行失败，返回码: {result.returncode}")
            print("trace: "+ input_file[2])
            print("错误输出:\n", result.stderr)
    except Exception as e:
        print(f"发生错误: {e}")
        print("trace: "+ input_file[2])

if __name__ == "__main__":
    c_program_path = "./ssd"  # 替换为你的C语言程序的路径
    trace_dir1 = "/mnt/d/SSD/LUN"
    trace_dir2 = "/mnt/d/SSD/msrc"
    trace_dir3 = "/mnt/d/SSD/Ali/2024"
    trace_dir4 = "/mnt/d/SSD/Ali"
    arg = [
       
    # ["1", f"{trace_dir1}/1613_0.csv", "4"],
    # ["1", f"{trace_dir1}/1708_0.csv", "4"],
    # ["1", f"{trace_dir2}/hm_0.csv", "4"],
    ["1", f"{trace_dir2}/prn_0.csv", "4"],
    ["1", f"{trace_dir3}/23.csv", "4"],
    # ["1", f"{trace_dir3}/213.csv", "4"],
    # ["1", f"{trace_dir4}/283_0_24.csv", "4"],
    ["1", f"{trace_dir4}/354_0_24.csv", "4"]

    ]
    result = subprocess.run(["make","clean"], shell=False, text=True, capture_output=True)
    print("程序输出:\n", result.stdout)
    result = subprocess.run(["make","all"], shell=False, text=True, capture_output=True)
    print("程序输出:\n", result.stdout)
    for i in arg:
        run_c_program(c_program_path,i)
