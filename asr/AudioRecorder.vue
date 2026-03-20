<template>
    <div>
      <h1>Audio Recorder</h1>
      <button @click="startRecording" :disabled="recording">Start Recording</button>
      <button @click="stopRecording" :disabled="!recording">Stop Recording</button>
      <div v-if="message">
        <strong>Server message:</strong> {{ message }}
      </div>
    </div>
  </template>
  
  <script setup>
  import { ref } from 'vue'
  
  const recording = ref(false)
  const message = ref('')
  let mediaRecorder = null
  let socket = null
  
  // Function to start capturing audio and sending it over WebSocket.
  const startRecording = async () => {
    try {
      // Request microphone access.
      const stream = await navigator.mediaDevices.getUserMedia({ audio: true })
      
      // Open a WebSocket connection to the FastAPI backend.
      socket = new WebSocket('ws://localhost:8000/ws')
      // Ensure binary messages are received as ArrayBuffer.
      socket.binaryType = 'arraybuffer'
      
      socket.onopen = () => {
        console.log('WebSocket connected')
      }
      socket.onmessage = (event) => {
        console.log('Message from server:', event.data)
        message.value = event.data
      }
      socket.onerror = (error) => {
        console.error('WebSocket error:', error)
      }
      socket.onclose = () => {
        console.log('WebSocket closed')
      }
      
      // Set up MediaRecorder to record audio.
      // The MIME type "audio/webm" is commonly supported; adjust if necessary.
      const options = { mimeType: 'audio/webm' }
      mediaRecorder = new MediaRecorder(stream, options)
      
      // Start recording with a timeslice (e.g., 250ms chunks).
      mediaRecorder.start(250)
      
      // When a data chunk is available, send it over WebSocket.
      mediaRecorder.ondataavailable = (e) => {
        if (e.data && e.data.size > 0 && socket.readyState === WebSocket.OPEN) {
          socket.send(e.data)
        }
      }
      
      recording.value = true
    } catch (err) {
      console.error('Error accessing microphone:', err)
    }
  }
  
  // Function to stop recording and close the connection.
  const stopRecording = () => {
    if (mediaRecorder && mediaRecorder.state !== 'inactive') {
      mediaRecorder.stop()
    }
    if (socket && socket.readyState === WebSocket.OPEN) {
      socket.close()
    }
    recording.value = false
  }
  </script>
  
  <style scoped>
  button {
    margin: 5px;
  }
  </style>
