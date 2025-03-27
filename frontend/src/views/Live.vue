<template>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Gidole&display=swap" rel="stylesheet">
    <v-container fluid background-color="surface" align="center">
        
        <!-- Cards Row -->
        <v-row class="d-flex justify-center mb-5" style="max-width: 1200px;">
            <v-col cols="2" v-for="(item, index) in cards" :key="index">
                <v-card class="hover-effect bold-card" color="primaryContainer" height="150px">
                    <v-card-item>
                        <v-card-title class="small-title">{{ item.title }}</v-card-title>
                        <br>
                        <span class="value">{{ item.value }}</span>
                        <br>
                        <span v-if="item.conversion" class="conversion">{{ item.conversion }}</span>
                    </v-card-item>
                </v-card>
            </v-col>
        </v-row>
        
        <!-- Graphs Row -->
        <v-row class="d-flex justify-start" style="max-width: 1200px;">
            <v-col cols="12">
                <v-card>
                    <v-card-title>Temperature Analysis (Live)</v-card-title>
                    <v-card-text>
                        <figure class="highcharts-figure">
                            <div id="container"></div>
                        </figure>
                    </v-card-text>
                </v-card>
            </v-col>
            
            <v-col cols="12" class="mt-5">
                <v-card>
                    <v-card-title>Humidity Analysis (Live)</v-card-title>
                    <v-card-text>
                        <figure class="highcharts-figure">
                            <div id="container1"></div>
                        </figure>
                    </v-card-text>
                </v-card>
            </v-col>
        </v-row>

        <!-- Moisture Table -->
        <v-row class="d-flex justify-center mt-5" style="max-width: 600px;">
            <v-col cols="12">
                <v-card>
                    <v-card-title>Moisture Changes</v-card-title>
                    <v-card-text>
                        <v-table>
                            <thead>
                                <tr>
                                    <th>Timestamp</th>
                                    <th>Moisture Increase</th>
                                    <th>Moisture Decrease</th>
                                </tr>
                            </thead>
                            <tbody>
                                <tr v-for="entry in moistureData" :key="entry.timestamp">
                                    <td>{{ entry.timestamp }}</td>
                                    <td>{{ entry.increase }}</td>
                                    <td>{{ entry.decrease }}</td>
                                </tr>
                            </tbody>
                        </v-table>
                    </v-card-text>
                </v-card>
            </v-col>
        </v-row>
    </v-container>
</template>


<script setup>
/** JAVASCRIPT HERE */

// IMPORTS
import { ref,reactive,watch ,onMounted,onBeforeUnmount,computed } from "vue";  
import { useRoute ,useRouter } from "vue-router";
import { useMqttStore } from "@/store/mqttStore"; 
import {storeToRefs } from "pinia";
import Highcharts from 'highcharts'; 
import more from 'highcharts/highcharts-more'; 
import Exporting from 'highcharts/modules/exporting'; 
Exporting(Highcharts);  
more(Highcharts); 



// VARIABLES
const router      = useRouter();
const route       = useRoute();  
const Mqtt = useMqttStore();
const {payload, payloadTopic } = storeToRefs(Mqtt);
const points  = ref(10); // Specify the quantity of points to be shown on the live graph simultaneously. 
const shift = ref(false); // Delete a point from the left side and append a new point to the right side of the graph. 
//const payload = ref({ temperature: 0, heatindex: 0, humidity: 0 });
const tempHiChart = ref(null); // Chart object
const HumidChart = ref(null); // Chart object

const CreateCharts = async () => { 
// TEMPERATURE CHART 
tempHiChart.value = Highcharts.chart('container', { 
chart: { zoomType: 'x' }, 
title: { text: 'Air Temperature and Heat Index Analysis', align: 'left' }, 
yAxis: {  
title: { text: 'Air Temperature & Heat Index' , style:{color:'#000000'}}, 
labels: { format: '{value} °C' }         
}, 
xAxis: { 
type: 'datetime',  
title: { text: 'Time', style:{color:'#000000'} }, 
}, 
tooltip: { shared:true, }, 
series: [ 
{ 
name: 'Temperature', 
type: 'spline', 
data: [], 
turboThreshold: 0, 
color: Highcharts.getOptions().colors[0] 
},  
{ 
name: 'Heat Index', 
type: 'spline', 
data: [], 
turboThreshold: 0, 
color: Highcharts.getOptions().colors[1] 
} ], 
}); 
 

HumidChart.value = Highcharts.chart('container1', { 
chart: { zoomType: 'x' }, 
title: { text: 'Humidity Analysis', align: 'left' }, 
yAxis: {  
title: { text: 'Humidity' , style:{color:'#2F3301'}}, 
labels: { format: '{value} °C' }         
}, 
xAxis: { 
type: 'datetime',  
title: { text: 'Time', style:{color:'#000000'} }, 
}, 
tooltip: { shared:true, }, 
series: [ 
{ 
name: 'Humidity', 
type: 'spline', 
data: [], 
turboThreshold: 0, 
color: Highcharts.getOptions().colors[0] 
}, ], 
}); 
}; 

// COMPUTED PROPERTIES
const cards = computed(() => [
    { title: "TEMPERATURE", value: `${payload.value.temperature.toFixed(2)} °C`, conversion: `${(payload.value.temperature * 9/5 + 32).toFixed(2)} °F` },
    { title: "HEAT INDEX", value: `${payload.value.heatindex.toFixed(2)} °C`, conversion: `${(payload.value.heatindex * 9/5 + 32).toFixed(2)} °F` },
    { title: "HUMIDITY", value: `${payload.value.humidity}%` },
    { title: "PRESSURE", value: `${payload.value.pressure.toFixed(2)} hPa`, conversion: `${(payload.value.pressure * 0.02953).toFixed(2)} inHg` },
    { title: "SOIL MOISTURE", value: `${payload.value.moisture.toFixed(2)} %` },
    { title: "ALTITUDE", value: `${payload.value.altitude.toFixed(2)} m`, conversion: `${(payload.value.altitude * 3.28084).toFixed(2)} ft` }
]);

const moistureData = ref([
    { timestamp: "12:00 PM", increase: "5%", decrease: "2%" },
    { timestamp: "01:00 PM", increase: "4%", decrease: "3%" },
]);
// FUNCTIONS
onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    CreateCharts();
    Mqtt.connect();
    setTimeout( ()=>{ Mqtt.subscribe("620164419")}, 3000);
});


onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    Mqtt.unsubcribeAll();
});

// WATCHERS 
watch(payload,(data)=> {   
if(points.value > 0){  points.value -- ; } 
else{ shift.value = true; }   
tempHiChart.value.series[0].addPoint({y:parseFloat(data.temperature.toFixed(2)) ,x: data.timestamp * 1000 }, 
true,  shift.value);  
tempHiChart.value.series[1].addPoint({y:parseFloat(data.heatindex.toFixed(2))
   ,x: data.timestamp * 1000 }, true,  shift.value);
HumidChart.value.series[0].addPoint({y:parseFloat(data.humidity.toFixed(2)) ,x: data.timestamp * 1000 }, 
true,  shift.value);  
});




</script>


<style scoped>
/** CSS STYLE HERE */
figure{
    margin: 2px;
}   
.small-title {
    font-size: 0.9em;
    font-weight: bold;
}
.hover-effect {
    transition: transform 0.3s ease-in-out;
}
.hover-effect:hover {
    transform: scale(1.05);
}
.value {
    font-weight: bold;
    font-size: 1.2em;
}
.conversion {
    font-size: 0.9em;
    opacity: 0.75;
}
</style>


  