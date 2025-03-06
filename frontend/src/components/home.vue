<script setup lang="ts">
import homeconfig from './homeconfig.vue'
</script>

<template>
  <homeconfig>
    <template #icon>
      <img src="../img/TeamIcon.png" class="Icon">
    </template>
    <template #heading>Team</template>
      <p class="text">
        Arthur Burjak
        <br>
        Leander Gastgeber
        <br>
        Jones Soliman
        <br>
        Mihael Stojkovic
      </p>
  </homeconfig>

  <homeconfig>
    <template #icon>
      <img src="../img/BetreuerIcon.png" class="Icon">
    </template>
    <template #heading>Betreuer</template>
      <p class="text">
        Herr Professor Dipl- Ing. Erich Erker
      </p>
  </homeconfig>

  <homeconfig>
    <template #icon>
      <img src="../img/IdeeIcon.png" class="Icon">
    </template>
    <template #heading>Idee</template>
      <p class="text">
        Ziel dieser Diplomarbeit ist es, drei Roboter zu entwickeln, 
        welche kooperativ die Umgebung erkunden können. 
        Hierbei verwenden wir einen LiDAR-Sensor
        ausgestattet, welcher Entfernungsmessungen durchführt. Die anderen beiden Roboter
        sollen komplett blind sein. Koordiniert wird das
        ganze über einen zentralen Server, welcher die gesammelten Daten zusätzlich über ein
        Webinterface darstellt.
      </p>
  </homeconfig>
</template>

<style>
.Icon{
  width: 50px;
  height: 45px;
}

.text{
  font-size: 1.1rem;
  font-weight: 450;
  margin-bottom: 0.2rem;
  color: black;
  display: flex;
  justify-content: left;
}
</style>
