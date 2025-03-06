<script lang="ts"> // indicates that we are using typescript
  import { ref, onMounted } from "vue" // standard import from vue for references  
  import * as ED from  '../lib/encoder_data'
  import * as message from  '../lib/encoder_data'
  import Chart from './charts.vue'

export default {
  components: {
    Chart,
  },
  setup(){
    const Nachricht = ref('') // reactive reference, used for input, empty string
    const output = ref([] as string[])  // stores the string from 'Nachricht'

  // initialize chart data
  const series = ref([{ name: 'Encoder Data', data: [] }]); 
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

  // eventlistener, when an event happens (message received), it is shown in the console
  socket.addEventListener("message", (event) => {
    output.value.push("server Nachricht", event.data);
    // deseralize binary data
    ED.at.htlw10.swarmbots.EncoderData
    const encoderData = ED.at.htlw10.swarmbots.EncoderData.deserializeBinary(event.data);
    // convert binary into something readable
    
    
    
    const pulses = message.pulses;
    const duration = message.duration;
    const msg = 'Pulses: ${pulses}, Duration: ${duration}';
    output.value.push(msg);

    // Update the chart data
  series.value[0].data.push(pulses); // Add new data point to the series
    if (series.value[0].data.length > 8) {
      series.value[0].data.shift(); // Remove the oldest data point if more than 8
    }
  });

  socket.onmessage = (event) => {
    output.value.push(event.data) //event data is pushed to the output for to display it in the ui
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