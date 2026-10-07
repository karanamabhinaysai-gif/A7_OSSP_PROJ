#!/usr/bin/env python3
"""
OSSP GitHub Web Terminal Server
Cross-platform: runs seamlessly in GitHub Codespaces (native Linux),
local Linux, or Windows WSL2.
"""

import os
import sys
import json
import time
import subprocess
from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

PORT = int(os.environ.get("PORT", 5050))
IS_WINDOWS = sys.platform == "win32"

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_DIR = os.path.dirname(BASE_DIR)
OSSP_ROOT = os.path.dirname(REPO_DIR)

SHELLFORGE_DIR = REPO_DIR
OS_PRATICAL_DIR = os.path.join(OSSP_ROOT, "OS_Pratical")
if not os.path.exists(OS_PRATICAL_DIR):
    OS_PRATICAL_DIR = REPO_DIR

if IS_WINDOWS:
    WSL_SHELLFORGE = "/mnt/e/KLH/2nd year/OSSP/A7_OSSP_PROJ"
    WSL_OS_PRATICAL = "/mnt/e/KLH/2nd year/OSSP/OS_Pratical"
    CACHED_GCC = "GCC (Ubuntu WSL2 / Windows 11)"
else:
    WSL_SHELLFORGE = SHELLFORGE_DIR
    WSL_OS_PRATICAL = OS_PRATICAL_DIR
    CACHED_GCC = "GCC (Native Linux / GitHub Codespaces)"

class TerminalRequestHandler(SimpleHTTPRequestHandler):
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path

        if path == "/" or path == "/index.html":
            self.serve_file(os.path.join(BASE_DIR, "index.html"), "text/html")
        elif path == "/api/status":
            self.handle_status()
        elif path == "/api/files":
            self.handle_files()
        elif path == "/api/file_content":
            query = parse_qs(parsed.query)
            repo = query.get("repo", ["shellforge"])[0]
            file_path = query.get("path", ["README.md"])[0]
            self.handle_file_content(repo, file_path)
        elif path == "/api/logs":
            self.handle_logs()
        else:
            super().do_GET()

    def do_POST(self):
        parsed = urlparse(self.path)
        if parsed.path == "/api/execute":
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length).decode("utf-8")
            try:
                data = json.loads(body)
                self.handle_execute(data)
            except Exception as e:
                self.send_json({"success": False, "error": str(e)}, status=400)
        else:
            self.send_response(404)
            self.end_headers()

    def send_json(self, data, status=200):
        response_bytes = json.dumps(data).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.send_header("Content-Length", str(len(response_bytes)))
        self.end_headers()
        self.wfile.write(response_bytes)

    def serve_file(self, file_path, content_type):
        if not os.path.exists(file_path):
            self.send_response(404)
            self.end_headers()
            return
        with open(file_path, "rb") as f:
            content = f.read()
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Content-Length", str(len(content)))
        self.end_headers()
        self.wfile.write(content)

    def handle_status(self):
        status_info = {
            "status": "online",
            "os": "Windows 11 / WSL2" if IS_WINDOWS else "Linux / GitHub Codespaces",
            "compiler": CACHED_GCC,
            "repositories": [
                {
                    "name": "ShellForge (A7_OSSP_PROJ)",
                    "path": SHELLFORGE_DIR,
                    "remote": "https://github.com/karanamabhinaysai-gif/A7_OSSP_PROJ",
                    "exists": os.path.exists(SHELLFORGE_DIR)
                },
                {
                    "name": "OS Practicals & Simulations",
                    "path": OS_PRATICAL_DIR,
                    "remote": "https://github.com/Srinath64312/os-practicals-and-simulations",
                    "exists": os.path.exists(OS_PRATICAL_DIR)
                }
            ]
        }
        self.send_json(status_info)

    def handle_files(self):
        result = {"shellforge": [], "os_pratical": []}

        if os.path.exists(SHELLFORGE_DIR):
            for root, _, files in os.walk(SHELLFORGE_DIR):
                if ".git" in root or "bin" in root or "logs" in root:
                    continue
                for f in files:
                    rel = os.path.relpath(os.path.join(root, f), SHELLFORGE_DIR).replace("\\", "/")
                    result["shellforge"].append(rel)

        if os.path.exists(OS_PRATICAL_DIR):
            for root, _, files in os.walk(OS_PRATICAL_DIR):
                if ".git" in root:
                    continue
                for f in files:
                    rel = os.path.relpath(os.path.join(root, f), OS_PRATICAL_DIR).replace("\\", "/")
                    result["os_pratical"].append(rel)

        result["shellforge"].sort()
        result["os_pratical"].sort()
        self.send_json(result)

    def handle_file_content(self, repo, rel_path):
        target_dir = SHELLFORGE_DIR if repo == "shellforge" else OS_PRATICAL_DIR
        full_path = os.path.normpath(os.path.join(target_dir, rel_path))

        if not full_path.startswith(target_dir) or not os.path.exists(full_path):
            self.send_json({"error": "File not found or permission denied"}, status=404)
            return

        try:
            with open(full_path, "r", encoding="utf-8", errors="replace") as f:
                content = f.read()
            self.send_json({"path": rel_path, "repo": repo, "content": content})
        except Exception as e:
            self.send_json({"error": str(e)}, status=500)

    def handle_logs(self):
        log_path = os.path.join(SHELLFORGE_DIR, "logs", "access.log")
        if not os.path.exists(log_path):
            self.send_json({"logs": "No logs recorded yet."})
            return
        try:
            with open(log_path, "r", encoding="utf-8", errors="replace") as f:
                lines = f.readlines()
            self.send_json({"logs": "".join(lines[-100:])})
        except Exception as e:
            self.send_json({"error": str(e)}, status=500)

    def handle_execute(self, data):
        repo = data.get("repo", "shellforge")
        cmd = data.get("command", "").strip()
        stdin_input = data.get("input", "")
        timeout = data.get("timeout", 20)
        client_cwd = data.get("cwd", "").strip()

        base_wsl_dir = WSL_SHELLFORGE if repo == "shellforge" else WSL_OS_PRATICAL
        current_wsl_dir = client_cwd if client_cwd else base_wsl_dir

        if not cmd:
            self.send_json({"success": False, "error": "No command specified"})
            return

        bash_script = f"{cmd}\n__RET=$?\necho '___OSSP_PWD___:'\npwd\nexit $__RET\n"

        start_time = time.time()
        try:
            if IS_WINDOWS:
                wsl_args = ["wsl", "--cd", current_wsl_dir, "bash", "-c", bash_script]
                proc = subprocess.run(
                    wsl_args,
                    input=stdin_input if stdin_input else None,
                    capture_output=True,
                    text=True,
                    timeout=timeout
                )
            else:
                proc = subprocess.run(
                    ["bash", "-c", bash_script],
                    cwd=current_wsl_dir if os.path.isdir(current_wsl_dir) else base_wsl_dir,
                    input=stdin_input if stdin_input else None,
                    capture_output=True,
                    text=True,
                    timeout=timeout
                )

            elapsed = time.time() - start_time

            raw_out = proc.stdout
            marker = "___OSSP_PWD___:\n"
            new_cwd = current_wsl_dir
            clean_out = raw_out

            if marker in raw_out:
                parts = raw_out.rsplit(marker, 1)
                clean_out = parts[0].rstrip("\r\n")
                candidate_cwd = parts[1].strip()
                if candidate_cwd:
                    new_cwd = candidate_cwd

            if new_cwd == base_wsl_dir:
                display_dir = "~"
            elif new_cwd.startswith(base_wsl_dir + "/"):
                display_dir = "~/" + new_cwd[len(base_wsl_dir) + 1:]
            else:
                display_dir = new_cwd

            self.send_json({
                "success": proc.returncode == 0,
                "exit_code": proc.returncode,
                "stdout": clean_out,
                "stderr": proc.stderr,
                "elapsed": round(elapsed, 3),
                "repo": repo,
                "command": cmd,
                "cwd": new_cwd,
                "display_dir": display_dir
            })
        except subprocess.TimeoutExpired:
            self.send_json({
                "success": False,
                "exit_code": -1,
                "stdout": "",
                "stderr": f"Execution timed out after {timeout} seconds.",
                "elapsed": timeout,
                "repo": repo,
                "command": cmd,
                "cwd": current_wsl_dir,
                "display_dir": "~"
            })
        except Exception as e:
            self.send_json({
                "success": False,
                "exit_code": -1,
                "stdout": "",
                "stderr": str(e),
                "elapsed": 0,
                "repo": repo,
                "command": cmd,
                "cwd": current_wsl_dir,
                "display_dir": "~"
            })

def main():
    print("=" * 60)
    print("  OSSP GitHub Web Terminal Server")
    print(f"  Mode: {'Windows WSL2' if IS_WINDOWS else 'Native Linux / Codespaces'}")
    print(f"  Serving on: http://localhost:{PORT}")
    print("=" * 60)
    server = ThreadingHTTPServer(("0.0.0.0", PORT), TerminalRequestHandler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server...")
        server.server_close()

if __name__ == "__main__":
    main()
