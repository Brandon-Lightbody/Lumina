import os
import sys
import platform
from pathlib import Path

def set_env_var(key, value):
    system = platform.system()

    if system == "Windows":
        # Set for current session
        os.system(f'setx {key} "{value}"')
        print(f"✅ Set {key} permanently in Windows environment.")
    else:
        shell = os.environ.get("SHELL", "")
        home = Path.home()

        # Determine shell config file
        if "zsh" in shell:
            rc_file = home / ".zshrc"
        else:
            rc_file = home / ".bashrc"

        export_line = f'export {key}="{value}"\n'

        # Avoid duplicate exports
        if rc_file.exists() and export_line.strip() in rc_file.read_text():
            print(f"ℹ️ {key} already set in {rc_file}")
        else:
            with open(rc_file, "a") as f:
                f.write(f"\n# Added by set_vcpkg_root.py\n{export_line}")
            print(f"✅ Appended {key} to {rc_file}")

        # Set for current session
        os.environ[key] = value
        print(f"✅ Set {key} for current session.")

def main():
    default_path = str(Path.home() / "vcpkg")
    vcpkg_path = input(f"Enter path to vcpkg [{default_path}]: ").strip() or default_path

    vcpkg_cmake = Path(vcpkg_path) / "scripts" / "buildsystems" / "vcpkg.cmake"
    if not vcpkg_cmake.exists():
        print("❌ That doesn't look like a valid vcpkg directory.")
        sys.exit(1)

    set_env_var("VCPKG_ROOT", str(Path(vcpkg_path).resolve()))

if __name__ == "__main__":
    main()
