import websocket

ws = websocket.WebSocket()
ws.connect('ws://localhost:10000/ws/server', header={'machine_id': 'test'})
ws.send('{"type": "init", "data": {"machine_id": "test"}}')