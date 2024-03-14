import os
import os
import sys

def remove_assets(directory):
    extensions = ['.blend', '.blend1', '.gltf', '.bin']
    
    for root, dirs, files in os.walk(directory):
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                file_path = os.path.join(root, file)
                os.remove(file_path)
                print(f"Removed asset: {file_path}")

# Example usage
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Please provide a directory as a command line argument.")
        sys.exit(1)
    
    directory = sys.argv[1]
    remove_assets(directory)
remove_assets(directory)