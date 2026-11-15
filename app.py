import os
import sys
import json
import shutil
import subprocess
import threading
from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

# Cross-platform binary naming
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
C_SOURCE = os.path.join(BASE_DIR, "dijkstra.c")
BINARY_NAME = "dijkstra.exe" if os.name == "nt" else "dijkstra"
BINARY_PATH = os.path.join(BASE_DIR, BINARY_NAME)
LOCATION_COUNT = 10


def open_in_chrome(url):
    """Open the local app in Chrome when Chrome is installed."""
    chrome = shutil.which("chrome.exe") or shutil.which("chrome")
    if not chrome:
        for program_dir in (os.environ.get("PROGRAMFILES"), os.environ.get("PROGRAMFILES(X86)"),
                            os.environ.get("LOCALAPPDATA")):
            if program_dir:
                candidate = os.path.join(program_dir, "Google", "Chrome", "Application", "chrome.exe")
                if os.path.isfile(candidate):
                    chrome = candidate
                    break

    if not chrome:
        print("[WARN] Google Chrome was not found. Set Chrome as the default browser to open terminal links in Chrome.")
        return

    try:
        subprocess.Popen([chrome, url])
    except OSError as error:
        print(f"[ERROR] Could not open Google Chrome: {error}")


def ensure_compiled():
    """Compiles dijkstra.c automatically if the binary is missing or outdated."""
    needs_compile = not os.path.exists(BINARY_PATH)
    if not needs_compile:
        # Recompile if C source was modified more recently than the binary
        if os.path.getmtime(C_SOURCE) > os.path.getmtime(BINARY_PATH):
            needs_compile = True

    if needs_compile:
        print(f"[INFO] Compiling {C_SOURCE} -> {BINARY_PATH}...")
        try:
            subprocess.run(
                ["gcc", C_SOURCE, "-o", BINARY_PATH],
                check=True,
                capture_output=True,
                text=True
            )
            print("[SUCCESS] C binary compiled successfully.")
        except FileNotFoundError:
            print("[ERROR] GCC compiler not found! Please install MinGW (Windows) or GCC (Linux/Mac).")
            sys.exit(1)
        except subprocess.CalledProcessError as e:
            print(f"[ERROR] Compilation failed:\n{e.stderr}")
            sys.exit(1)


@app.route("/")
def index():
    """Serves the main interactive campus map UI."""
    return render_template("index.html")


@app.route("/route", methods=["GET", "POST"])
def find_route():
    """Executes the compiled C binary and returns the JSON shortest path."""
    if request.method == "POST":
        data = request.get_json(silent=True) or {}
        if not isinstance(data, dict):
            return jsonify({"error": "Request body must be a JSON object."}), 400
        src = data.get("src")
        dest = data.get("dest")
    else:
        src = request.args.get("src")
        dest = request.args.get("dest")

    if src is None or dest is None:
        return jsonify({"error": "Missing 'src' or 'dest' parameter."}), 400

    def parse_index(value):
        if isinstance(value, bool) or not isinstance(value, (int, str)):
            return None
        try:
            index = int(value)
        except ValueError:
            return None
        return index if 0 <= index < LOCATION_COUNT else None

    src_int = parse_index(src)
    dest_int = parse_index(dest)
    if src_int is None or dest_int is None:
        return jsonify({
            "error": "Source and destination must be integer location indices from 0 to 9."
        }), 400

    try:
        # Execute the compiled C binary with src and dest arguments
        result = subprocess.run(
            [BINARY_PATH, str(src_int), str(dest_int)],
            capture_output=True,
            text=True,
            check=True
        )
        route_data = json.loads(result.stdout)
        return jsonify(route_data)

    except subprocess.CalledProcessError as e:
        return jsonify({"error": "C program execution failed.", "details": e.stderr}), 500
    except json.JSONDecodeError:
        return jsonify({"error": "Failed to parse JSON output from C binary."}), 500


if __name__ == "__main__":
    ensure_compiled()
    url = "http://127.0.0.1:5000"
    print(f"[INFO] Starting Smart Campus Route Finder at {url}")
    if os.environ.get("WERKZEUG_RUN_MAIN") == "true":
        threading.Timer(1, open_in_chrome, args=(url,)).start()
    app.run(debug=True, port=5000)