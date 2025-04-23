<script lang="ts">
import VueApexCharts from 'vue3-apexcharts'
import pulses from './daten.vue'

export default {
  name: 'Chart',
  components: {
    apexcharts: VueApexCharts,
  },
  data: function() {
    return {
      encoderOptions: {
        chart: {
          id: 'encoder_Chart'
        },
        xaxis: {
          title: {
            text: 't [s]',
            style: {
            fontSize:  '16px',
          }  
          },
          categories: [1, 2, 3, 4, 5, 6, 7, 8]
        },
        yaxis: {
          title: {
            text: 'v [m/s]',
            style: {
            fontSize:  '16px',
          }
          }
        },
        title: {
          text: 'Encoder Chart',
          align: 'center',
          style: {
            fontSize:  '20px',
          }
        }
      },
      // Hier wird die Pulszahl importiert und als Datenreihe verwendet
      encoderSeries: [{
        name: 'series-1',
        data: [pulses] // Beispielwerte [0,2,5,2,1,3,4]
      }],
      imuOptions: {
        chart: {
          id: 'imu_Chart'
        },
        xaxis: {
          title: {
            text: 't [s]',
            style: {
              fontSize:  '16px',
            }  
          },
          categories: [1, 2, 3, 4, 5, 6, 7, 8]
        },
        yaxis: {
          title: {
            text: 'v [m/s]',
            style: {
            fontSize:  '16px',
          }
          }
        },
        title: {
          text: 'Imu Chart',
          align: 'center',
          style: {
            fontSize:  '20px',
          }
        }
      },
      imuSeries: [{
        name: 'series-1',
        data: [pulses] // Beispielwerte [0,2,5,2,1,3,4]
      }]
    }
  },
}
</script>

<template>
  <section class="Charts">
    <div class="encoder_Chart">
    <apexcharts width="500" type="line" :options="encoderOptions" :series="encoderSeries"></apexcharts>
    </div>
    <div class="imu_Chart">
    <apexcharts width="500" type="line" :options="imuOptions" :series="imuSeries"></apexcharts>
    </div>
  </section>
</template>

<style lang="css">
  .Charts{
    display: flex;
    margin-top: 4rem;
  }
  .encoder_Chart{
    margin-left: auto;
    margin-right: auto;
  }
  .imu_Chart{
    margin-left: 300px;
    margin-right: auto;
  }
</style>
