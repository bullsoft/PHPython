<?php
$code = <<<EOD

from http.server import BaseHTTPRequestHandler, HTTPServer
import json

class testHTTPServer_RequestHandler(BaseHTTPRequestHandler):
    # GET
    def do_GET(self):
       # Send response status code
       self.send_response(200)

       # Send headers
       self.send_header('Content-type','text/json')
       self.end_headers()

       # Send message back to client
       message = json.dumps(doGET(self.path))
       # Write content as utf-8 data
       self.wfile.write(bytes(message, "utf8"))
       return

def run():
    print('starting server...')

    # Server settings
    # Choose port 8080, for port 80, which is normally used for a http server, you need root access
    server_address = ('127.0.0.1', 8081)
    httpd = HTTPServer(server_address, testHTTPServer_RequestHandler)
    print('running server...')
    httpd.serve_forever()


run()
EOD;

$python = new Python();

$python->def("doGET", function($params){
    return [
        "a" => $params,
        "b" => "知道",
    ];
});

$python->eval($code);
