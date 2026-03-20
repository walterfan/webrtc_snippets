from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
import uvicorn

app = FastAPI()

# Allow requests from our local development origins
origins = [
    "http://localhost:3000",  # Vue dev server default port (or adjust as needed)
    "http://localhost:8080",
    "http://localhost:8000",
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,  # or ["*"] for all origins in development
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            # Receive audio data as binary (bytes)
            data = await websocket.receive_bytes()
            print("Received audio data, size:", len(data))
            # Send a confirmation message back to the client
            await websocket.send_text("Audio packet received")
    except WebSocketDisconnect:
        print("Client disconnected")

if __name__ == '__main__':
    uvicorn.run(app, host="0.0.0.0", port=8000)
