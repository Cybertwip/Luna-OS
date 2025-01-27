import subprocess
import sys

def format_disk(disk_img_path):
    # Attach the disk image without mounting
    attach_cmd = f"hdiutil attach -nomount {disk_img_path}"
    try:
        result = subprocess.run(attach_cmd, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        device = result.stdout.split()[0]  # Extract the device identifier
    except subprocess.CalledProcessError as e:
        print(f"Error attaching disk image: {e}", file=sys.stderr)
        return 1
    
    # Format the device as FAT32
    format_cmd = f"newfs_msdos -F 32 {device}"
    try:
        subprocess.run(format_cmd, shell=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error formatting disk: {e}", file=sys.stderr)
        # Detach the device on error to clean up
        subprocess.run(f"hdiutil detach {device}", shell=True)
        return 1

    # Detach the device
    detach_cmd = f"hdiutil detach {device}"
    try:
        subprocess.run(detach_cmd, shell=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error detaching disk image: {e}", file=sys.stderr)
        return 1

    return 0

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python format_disk.py <path_to_disk_image>", file=sys.stderr)
        sys.exit(1)
    
    disk_img_path = sys.argv[1]
    sys.exit(format_disk(disk_img_path))