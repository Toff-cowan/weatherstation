<template>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Gidole&display=swap" rel="stylesheet">
    <v-container fluid background-color="surface" align="center">
        
        <!-- Cards Row -->
        <v-row class="d-flex justify-center mb-5" style="max-width: 1200px;">
            <v-col cols="2" v-for="(item, index) in cards" :key="index">
                <div v-if="item.title === 'HUMIDITY' || item.title === 'SOIL MOISTURE'" class="moisture-card hover-effect">
                    <v-card class="bold-card" color="primaryContainer" height="150px">
                        <v-card-item>
                            <v-icon size="30">{{ item.icon }}</v-icon>
                            <v-card-title class="small-title">{{ item.title }}</v-card-title>
                            <br>
                            <span class="value">{{ item.value }}</span>
                        </v-card-item>
                    </v-card>
                    <div v-if="item.title === 'SOIL MOISTURE'" class="wave"></div>
                </div>
                <div v-else class="flip-card">
                    <div class="flip-card-inner">
                        <div class="flip-card-front">
                            <v-card class="hover-effect bold-card" color="primaryContainer" height="150px">
                                <v-card-item>
                                    <v-icon size="30">{{ item.icon }}</v-icon>
                                    <v-card-title class="small-title">{{ item.title }}</v-card-title>
                                    <br>
                                    <span class="value">{{ item.value }}</span>
                                </v-card-item>
                            </v-card>
                        </div>
                        <div class="flip-card-back">
                            <v-card class="hover-effect bold-card" color="primaryContainer" height="150px">
                                <v-card-item>
                                    <v-icon size="30">{{ item.icon }}</v-icon>
                                    <v-card-title class="small-title">{{ item.title }}</v-card-title>
                                    <br>
                                    <span class="value">{{ item.conversion }}</span>
                                </v-card-item>
                            </v-card>
                        </div>
                    </div>
                </div>
            </v-col>
        </v-row>
        
        <!-- Graphs Row -->
        <v-row class="d-flex justify-center" style="max-width: 1200px;">
    <v-col cols="6">
        <v-card>
            <v-card-title>Temperature Analysis (Live)</v-card-title>
            <v-card-text>
                <figure class="highcharts-figure">
                    <div id="container"></div>
                </figure>
            </v-card-text>
        </v-card>
    </v-col>
    
    <v-col cols="6">
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
    <v-table style="max-height: 300px; overflow-y: auto; display: block;">
        <thead>
            <tr>
                <th>Timestamp</th>
                <th>Moisture Increase</th>
                <th>Moisture Decrease</th>
            </tr>
        </thead>
        <tbody>
            <tr v-for="entry in visibleMoistureData" :key="entry.timestamp">
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
const moistureHistory = ref([]);

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
    { title: "TEMPERATURE", value: `${payload.value.temperature.toFixed(2)} °C`, conversion: `${(payload.value.temperature * 9/5 + 32).toFixed(2)} °F`, icon: "mdi-thermometer" },
    { title: "HEAT INDEX", value: `${payload.value.heatindex.toFixed(2)} °C`, conversion: `${(payload.value.heatindex * 9/5 + 32).toFixed(2)} °F`, icon: "mdi-fire" },
    { title: "HUMIDITY", value: `${payload.value.humidity}%`, icon: "mdi-water-percent" },
    { title: "PRESSURE", value: `${payload.value.pressure.toFixed(2)} hPa`, conversion: `${(payload.value.pressure * 0.02953).toFixed(2)} inHg`, icon: "mdi-gauge" },
    { title: "SOIL MOISTURE", value: `${payload.value.moisture.toFixed(2)} %`, conversion: "Moisture level", icon: "mdi-water" },
    { title: "ALTITUDE", value: `${payload.value.altitude.toFixed(2)} m`, conversion: `${(payload.value.altitude * 3.28084).toFixed(2)} ft`, icon: "mdi-mountain" }
]);

const moistureData = computed(() => {
    if (moistureHistory.value.length < 2) return [];

    let lastIncrease = 0;
    let lastDecrease = 0;

    return moistureHistory.value.map((entry, index, arr) => {
        if (index === 0) {
            return {
                timestamp: new Date(entry.timestamp * 1000).toLocaleTimeString(),
                increase: lastIncrease,
                decrease: lastDecrease,
            };
        }

        const prevMoisture = arr[index - 1].moisture;
        const moistureDiff = entry.moisture - prevMoisture;

        if (moistureDiff > 0) {
            lastIncrease = `${moistureDiff.toFixed(2)}%`;
            lastDecrease = "-";
        } else if (moistureDiff < 0) {
            lastDecrease = `${Math.abs(moistureDiff).toFixed(2)}%`;
            lastIncrease = "-";
        }

        return {
            timestamp: new Date(entry.timestamp * 10000).toLocaleTimeString(),
            increase: lastIncrease,
            decrease: lastDecrease,
        };
    });
});
const visibleMoistureData = computed(() => {
    if (moistureHistory.value.length < 3) return moistureData.value;
    return moistureData.value.slice(-3); // Show only the last 3 entries
});
// FUNCTIONS
onMounted(()=>{
    // THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    CreateCharts();
    Mqtt.connect();
    setTimeout( ()=>{ Mqtt.subscribe("620164419")}, 3000);
    setTimeout(() => {
        alert("The page will refresh in  5 minutes");
    }, 1000);
    setInterval(() => {
        location.reload();
    }, 300000);

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
watch(payload, (data) => {
    const currentTimestamp = Math.floor(Date.now() / 1000); // Get current time in seconds

    if (moistureHistory.value.length > 0) {
        console.log(
            `Previous Moisture: ${moistureHistory.value[moistureHistory.value.length - 1].moisture}, ` +
            `New Moisture: ${data.moisture}`
        );
    }

    // Store new moisture reading
    moistureHistory.value.push({
        timestamp: currentTimestamp,
        moisture: parseFloat(data.moisture.toFixed(2))
    });

    // Keep only the last 100 records
    if (moistureHistory.value.length > 100) {
        moistureHistory.value.shift();
    }
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
.flip-card {
    perspective: 1000px;
}

.flip-card-inner {
    width: 100%;
    height: 150px;
    transition: transform 0.6s;
    transform-style: preserve-3d;
    position: relative;
}

.flip-card:hover .flip-card-inner {
    transform: rotateY(180deg);
}

.flip-card-front, .flip-card-back {
    position: absolute;
    width: 100%;
    height: 100%;
    backface-visibility: hidden;
}

.flip-card-back {
    transform: rotateY(180deg);
}

.moisture-card {
    position: relative;
    overflow: hidden;
}

.wave {
    position: absolute;
    bottom: 0;
    left: 0;
    width: 100%;
    height: 20%;
    background: rgba(0, 150, 255, 0.5);
    animation: wave-animation 2s infinite ease-in-out alternate;
}

@keyframes wave-animation {
    0% { height: 20%; }
    100% { height: 40%; }
}
</style>


  