<?php
// It requires Python ≥ 3.4 or Python 3.3 with the asyncio module
// websockets https://websockets.readthedocs.io/en/stable/
// pip install asyncio
// pip install websockets


dl('python.so');

$code = <<<EOD

import asyncio
import websockets

async def hello(websocket, path):
    name = await websocket.recv()
    print("< {}".format(name))

    #greeting = "Hello {}!".format(name)
    greeting = "Hello " +  phpformat(name)
    await websocket.send(greeting)
    print("> ", greeting)
    #phpformat(name)

start_server = websockets.serve(hello, 'localhost', 8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()

EOD;

$python = new Python();
$python->def("phpformat", function($params) {
    $name = reset($params);
    return "PHP Says: Hello " . $name;
});
try {
    $python->eval($code);
} catch(\Exception $e) {
    echo "get exception";
}