#pragma once

float NormalizeInputToDeadzone(int16_t val, int16_t threshold) {
	float normalized = 0;

	if (val < threshold) {
		normalized = (float)(val + threshold) / (32768.0f - threshold); 
	} else if (val > threshold) {
		normalized = (float)(val - threshold) / (32768.0f - threshold); 
	}

	return normalized;
}

float SDLProcessGameControllerAxisValue(int16_t Value, int16_t DeadZoneThreshold) {
	float result = 0;
	if(Value < -DeadZoneThreshold) {
        result = (float)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    } else if(Value > DeadZoneThreshold) {
        result = (float)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }

    return(result);
}