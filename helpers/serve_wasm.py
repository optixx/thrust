#!/usr/bin/env python3
import argparse
import http.server
import os
import socketserver


def main():
    parser = argparse.ArgumentParser(description="Serve the wasm build directory over HTTP.")
    parser.add_argument("--directory", "-d", default="wasm", help="Directory to serve (default %(default)s)")
    parser.add_argument("--port", "-p", type=int, default=8000, help="Port to listen on (default %(default)s)")
    args = parser.parse_args()

    os.chdir(args.directory)
    handler = http.server.SimpleHTTPRequestHandler
    with socketserver.TCPServer(("", args.port), handler) as httpd:
        print(f"Serving {args.directory} at http://localhost:{args.port}/")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server.")
            httpd.server_close()


if __name__ == "__main__":
    main()
