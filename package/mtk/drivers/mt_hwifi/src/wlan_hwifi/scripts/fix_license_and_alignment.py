import json
import os
import re
import subprocess

json_file_name = "summary.json"

def text_exist(file_path, regex, regex_flags):
    with open(file_path, "r") as f:
        read_text = f.read()
        found = re.search(regex, read_text, regex_flags)

        if found:
            return found.group(0)
        else:
            return None

def get_block_comment_regex():
    return "^\/\*((?!\*\/).|\n)+\*\/$"

def block_comment_text_exist(file_path, regex):
    return text_exist(file_path, regex, re.MULTILINE)

def write_aligned_text(file_path):
    modified_text = ""
    
    with open(file_path, "r") as f:
        read_text = f.read()
        modified_text = re.sub("^\*", " *", read_text, flags = re.MULTILINE)
        
    with open(file_path, "w") as f:
        f.write(modified_text)

def get_license_regex():
    return "^(\/\*|\/\/) SPDX-License-Identifier: .+( \*\/)?$"

def license_text_exist(file_path, regex):
    return text_exist(file_path, regex, re.MULTILINE)

def get_license_id(text, regex):
    license_tag_id = re.search(regex, text, re.MULTILINE)
    license_tag_id = license_tag_id.group(0).split(" ", 2)
    
    if len(license_tag_id) < 3:
        return None
    else:
        license_id = license_tag_id[2].rstrip(" /*")
        return license_id

def write_license_text(file_path, license_id):
    modified_text = ""
    
    with open(file_path, "r") as f:
        read_text = f.read()

        if file_path.endswith(".c"):
            modified_text = ("// SPDX-License-Identifier: %s\n" % license_id) + read_text
        elif file_path.endswith(".h"):
            modified_text = ("/* SPDX-License-Identifier: %s */\n" % license_id) + read_text
        else:
            modified_text = read_text

    with open(file_path, "w") as f:
        f.write(modified_text)

def modify_license_text(file_path, regex, license_id):
    modified_text = ""
    
    with open(file_path, "r") as f:
        read_text = f.read()

        if file_path.endswith(".c"):
            modified_text = re.sub(regex, "// SPDX-License-Identifier: %s" % license_id, read_text, flags = re.MULTILINE)
        elif file_path.endswith(".h"):
            modified_text = re.sub(regex, "/* SPDX-License-Identifier: %s */" % license_id, read_text, flags = re.MULTILINE)
        else:
            modified_text = read_text

    with open(file_path, "w") as f:
        f.write(modified_text)

if os.getcwd().endswith("wlan_hwifi"):
    license_id = input("Please input SPDX License ID: ")

    with open(json_file_name, "r") as json_file:
        summary = json.load(json_file)

        for file_path in summary:
            if "All" in file_path:
                continue

            print(file_path)

            block_comment_regex = get_block_comment_regex()
            block_comment_text = block_comment_text_exist(file_path, block_comment_regex)

            if block_comment_text:
                write_aligned_text(file_path)
                print("\tAligned block comment")

            if len(license_id) < 1:
                print("\tSPDX License ID is empty, skip check")
                continue

            license_regex = get_license_regex()
            license_text = license_text_exist(file_path, license_regex)
            
            if license_text is None:
                write_license_text(file_path, license_id)
                print("\tWrote SPDX License ID")
            elif license_id != get_license_id(license_text, license_regex):
                modify_license_text (file_path, license_regex, license_id)
                print("\tModifed SPDX License ID")

else:
    print("Please run command within \"wlan_hwifi\" directory")