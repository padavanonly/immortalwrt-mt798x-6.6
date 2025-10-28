import json
import os
import subprocess

json_file_name = "summary.json"
txt_file_name = "warn_err_mesg.txt"
error_count = 0
warning_count = 0

if os.getcwd().endswith("wlan_hwifi"):
    with open(txt_file_name, "w") as txt_file:
        with open(json_file_name, "r") as json_file:
            summary = json.load(json_file)
            
            for file_path in summary:
                if "All" in file_path:
                    continue

                # run_cmd = ["/usr/src/linux-headers-3.16.0-30/scripts/checkpatch.pl", "-f", "--no-tree", file_path]
                run_cmd = ["./script/checkpatch.pl", "-f", "--no-tree", file_path]
                
                try:
                    subprocess.check_output(run_cmd)
                except subprocess.CalledProcessError as e:
                    print ("Filepath: %s" % file_path)
                    txt_file.write(e.output.decode())

                    output_arr = e.output.decode().split("\n")
                    
                    for output in output_arr:
                        if "total:" in output:
                            total_mesg = output.split(" ")
                            
                            if len(total_mesg) < 8:
                                print("\t***%s***" % output) 
                                continue
                            else:
                                error_count   += int(total_mesg[1])
                                warning_count += int(total_mesg[3])
        
        print("Data is written into %s" % txt_file_name)
        print("Warning count = %6d, Error count = %6d" % (warning_count, error_count))

else:
    print("Please run command within \"wlan_hwifi\" directory")