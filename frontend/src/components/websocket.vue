<script lang="ts"> // indicates that we are using typescript
import { ref } from "vue" // standard import from vue for references  
import { EncoderData } from  '../lib/encoder_data'
import Chart from './charts.vue'

export default {
  name: "Websocket",
  components: {
    Chart,
  },
  setup(){
    const Nachricht = ref('') // reactive reference, used for input, empty string
    const output = ref([] as string[])  // stores the string from 'Nachricht'
    const series = ref<{ name: string; data: number[] }[]>([
      { name: "Encoder Data", data: [] },
    ]);

  // initialize chart data
    const chartOptions = ref({
      chart: {
        id: 'basic-bar',
      },
      xaxis: {
        categories: [1, 2, 3, 4, 5, 6, 7, 8], 
      },
    });

  const socket = new WebSocket('ws://172.16.4.35:8080')

  socket.onopen = () => {
    console.log("Verbunden")
    }

  socket.onclose = () => {
    console.log("Geschlossen")
    }
    
    socket.binaryType = "arraybuffer";


    // auf ne typescript datei auslagern
    socket.addEventListener("message", (event) => {
      try {
      const binaryData = new Uint8Array(event.data); // Konvertiere ArrayBuffer zu Uint8Array

        // deseralize data
        const decodedData = EncoderData.fromBinary(binaryData);

        // extract pulses and duration from EncoderData
        const pulses = decodedData.pulses;
        const duration = decodedData.duration;

        // output
        console.log(`Pulses: ${pulses}, Duration: ${duration} ms`);
        output.value.push(`Pulses: ${pulses}, Duration: ${duration} ms`);

        // update chart
        series.value[0].data.push(pulses);
        if (series.value[0].data.length > 8) {
          series.value[0].data.shift(); // Entferne den ältesten Datenpunkt, wenn mehr als 8 vorhanden sind
        }
      }
        catch (error) {
        console.error("Fehler beim Deserialisieren der Nachricht:", error);
      }
    });

  socket.onmessage = (event) => {
    output.value.push(event.data) //event data is pushed to the output for display it in the ui
    }

    return {
      Nachricht,
      output,
      socket,
      series,
      chartOptions,
    };
  }
};
</script>