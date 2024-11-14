#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#define DEFAULT_SETTINGS "              \
{                                       \
    \"outputSettings\":{                \
                \"propulsionSystem\":{  \
            \"armChannel\": 0,         \
            \"motors\":[                \
                {                       \
                    \"channel\": 1,    \
                    \"polarity\": 0,    \
                    \"minPower\": 20,   \
                    \"maxPower\": 100,   \
                    \"responseTime\": 1000 \
                },                      \
                                {       \
                    \"channel\": 2,     \
                    \"polarity\": 1,    \
                    \"minPower\": 20,   \
                    \"maxPower\": 100,   \
                    \"responseTime\": 1000 \
                },                      \
                {                       \
                    \"channel\": 3,     \
                    \"polarity\": 1,    \
                    \"minPower\": 0,   \
                    \"maxPower\": 75,   \
                    \"responseTime\": 1000 \
                },                      \
                {                       \
                    \"channel\": 4,     \
                    \"polarity\": 0,    \
                    \"minPower\": 0,   \
                    \"maxPower\": 80,   \
                    \"responseTime\": 1000 \
                }                       \
            ]                           \
        },                              \
        \"battery\":{                   \
            \"maxVoltage\": 12.6,       \
            \"minVoltage\": 7.5         \
        },                              \
        \"sensors\":{                   \
            \"updateInterval\": 50,    \
            \"currentFactor\": 0.00805861,     \
            \"voltageFactor\": 0.00321676      \
        },                              \
        \"accessories\":{               \
            \"ledChannel\": 4,          \
            \"maxLedPower\": 75         \
        }                               \
        }                               \
}                                       \
"                                       \

/* SENSOR_DATA "                   \
{                                       \
    \"sensors\":{                       \
        \"batteryVoltage\":0.00,         \
        \"motorCurrent\":0.00,           \
        \"outputs\":[0,0,0,0,0,0,0,0]   \
  }                                     \
}                                       \
"                                       \
*/
#endif
