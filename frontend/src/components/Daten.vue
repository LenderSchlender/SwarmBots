<script setup lang="ts">
import socket from './websocket.vue'
import { ref } from "vue" // standard import from vue for references
import Chart from './charts.vue'
import Map from './map.vue'

const Nachricht = ref('') // reactive reference, used for input, empty string
const output = ref([] as string[])  // stores the string from 'Nachricht'

function senden() {
    if(Nachricht.value.trim() !== '') // if empty nothing gets send
      {
      socket.send(Nachricht.value) // message to the Websocket
      output.value.push(Nachricht.value)  // message to the output as test
      Nachricht.value ='' // clears input
      }
    }
</script>

<template>
  <!--v-model = textbox, changes effect 'Nachricht'-->
<input v-model="Nachricht" placeholder="Schreiben Sie an den Server" class="WebsocketBox"/>  
<button @click="senden" class="WebsocketBTN">Senden</button>
  <p>
    <!--:key="msg" provides a unique identifier for, helps vue track it more efficiently-->
    <span v-for="msg in output":key="msg" class="WebsocketOUT">{{ msg }}<br></span>
  </p> 
  <p>
    <Chart />
  </p>
  <p>
    <Map />
  </p>
</template>

<style lang="css">
  .WebsocketBox{
      background-color: whitesmoke;
      height: 1.8rem;
      width: 250px;
      max-width: 250px;
      font-size: 1rem;
      text-align: center;
      margin-top: 10px;
    }
  .WebsocketBTN{
      position: relative;
      height: 1.82rem;
    }
  .WebsocketOUT{
      position: relative;
      color: black;
    }
</style> 
