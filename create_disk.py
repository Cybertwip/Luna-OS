import os
import subprocess

def create_fat32_disk(image_path="disk.img", size_gb=1):
    # Delete existing image if present
    if os.path.exists(image_path):
        os.remove(image_path)
    
    # Step 1: Create a raw disk image with dd
    print("Creating raw disk image...")
    image_size = size_gb * 1024 * 1024 * 1024  # 1GB in bytes
    with open(image_path, "wb") as f:
        f.truncate(image_size)
    
    # Step 2: Attach the raw image
    print("Attaching disk image...")
    attach_info = subprocess.check_output(
        ["hdiutil", "attach", "-nomount", image_path]
    ).decode().strip()
    disk_device = attach_info.split()[0]  # e.g., /dev/disk4

    try:
        # Step 3: Partition the disk with MBR and FAT32
        print("Partitioning...")
        subprocess.run(
            ["diskutil", "partitionDisk", disk_device, "1", "MBR", "MS-DOS", "LUNASYS", "100%"],
            check=True
        )
    finally:
        # Step 4: Detach the disk image
        subprocess.run(["hdiutil", "detach", disk_device], check=True)

    print(f"FAT32 disk image created at {image_path}")

if __name__ == "__main__":
    create_fat32_disk()