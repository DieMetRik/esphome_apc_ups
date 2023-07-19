//UPS APC ESPhome
//My telegram https://t.me/DieMetRik

#include "esphome.h"
#include "esphome/core/log.h"
#include "EEPROM.h"
#define EEPROM_SIZE 8



class Upcapc : public PollingComponent, public UARTDevice {
	Sensor *Estimated_runtime {nullptr};
	Sensor *Input_voltage {nullptr};
	Sensor *Temperature {nullptr};
	Sensor *Battery_level {nullptr};
	Sensor *Power_load {nullptr};
	Sensor *Output_voltage {nullptr};
	Sensor *Battery_voltage {nullptr};
	Sensor *Line_frequency {nullptr};
	BinarySensor *Runtime_calibration {nullptr};
	BinarySensor *Smart_trim {nullptr};
	BinarySensor *Smart_boost {nullptr};
	BinarySensor *On_line {nullptr};
	BinarySensor *On_battery {nullptr};
	BinarySensor *Overloaded_output {nullptr};
	BinarySensor *Battery_low {nullptr};
	BinarySensor *Replace_battery {nullptr};
	TextSensor *Last_cause {nullptr};
	TextSensor *Status {nullptr};
	Sensor *WorkOnBattery_count {nullptr};
	
	public:
	Upcapc(
	UARTComponent *parent, 
	Sensor *sensor1, 
	Sensor *sensor2, 
	Sensor *sensor3, 
	Sensor *sensor4, 
	Sensor *sensor5, 
	Sensor *sensor6, 
	Sensor *sensor7, 
	Sensor *sensor8,
	BinarySensor *sensor9,
	BinarySensor *sensor10,
	BinarySensor *sensor11,
	BinarySensor *sensor12,
	BinarySensor *sensor13,
	BinarySensor *sensor14,
	BinarySensor *sensor15,
	BinarySensor *sensor16,
	TextSensor *sensor17,
	TextSensor *sensor18,
	Sensor *sensor19
	) : UARTDevice(parent), 
	Estimated_runtime(sensor1),
	Input_voltage(sensor2),
	Temperature(sensor3),
	Battery_level(sensor4),
	Power_load(sensor5),
	Output_voltage(sensor6),
	Battery_voltage(sensor7),
	Line_frequency(sensor8),
	Runtime_calibration(sensor9),
	Smart_trim(sensor10),
	Smart_boost(sensor11),
	On_line(sensor12),
	On_battery(sensor13),
	Overloaded_output(sensor14),
	Battery_low(sensor15),
	Replace_battery(sensor16),
	Last_cause(sensor17),
	Status(sensor18),
	WorkOnBattery_count(sensor19){}

//common
	unsigned char params_smart_mode[1];			// Y
	unsigned char params_status[1];				// Q
//bool
	unsigned char params_bye[1];				// R

//double
	unsigned char params_input_voltage[1];		// L
	unsigned char params_temperature[1];		// C
	unsigned char params_battery_level[1];		// f
	unsigned char params_power_load[1];			// P
	unsigned char params_output_voltage[1];		// o
	unsigned char params_battery_voltage[1]; 	// B
	unsigned char params_estimated_runtime[1]; 	// j
	unsigned char params_line_frequency[1]; 	// F 
//Text sensor
	unsigned char params_last_cause[1]; 		// G
	
	uint8_t Re_buf[40];
	int counter = 0;
	
	String status = "OFFLINE";
	
	int error_cnt = 0;
	int total_error_cnt = 0;
	int step = 0;
	int prev_step = 0;

	int interval = 1000; 		// ИНТЕРВАЛ ОБНОВЛЕНИЯ

	bool wrk_on_btr = 0;
	bool wrk_on_btr_trig = 0;
	
	int address = 0;
	int32_t workonbattery_count;
	

//=======================================================
	void calculateParams(unsigned char *frame, unsigned char comm)
	{
		frame[0] = comm;

	}
//=======================================================
	double byteToFloat (uint8_t buf[], int cnt)
	{
		uint8_t tmp_buf[10];
		for (int i = 0;i<cnt; i++){
			tmp_buf[i]= buf[i];
		}
		String str1 = (char*)tmp_buf;
		double myDouble = str1.toDouble();
		return myDouble;
		
	}
	
	int byteToInt (uint8_t buf[], int cnt)
	{
		uint8_t tmp_buf[10];
		for (int i = 0;i<cnt; i++){
			tmp_buf[i]= buf[i];
		}
		String str1 = (char*)tmp_buf;
		int myInt = str1.toInt();
		return myInt;
		
	}
//=======================================================
	void setup() override
	{
		this->set_update_interval(interval);	
		calculateParams(params_estimated_runtime, 0x6a); 	// j 
		calculateParams(params_input_voltage, 0x4c);		// L
		calculateParams(params_temperature, 0x43); 			// C
		calculateParams(params_battery_level, 0x66);		// f
		calculateParams(params_power_load, 0x50);			// P in %
		calculateParams(params_output_voltage, 0x6f);		// o
		calculateParams(params_battery_voltage, 0x42);		// B
		calculateParams(params_line_frequency, 0x46); 		// F 

		calculateParams(params_status, 0x51);				// Q
		calculateParams(params_last_cause, 0x47); 			// G
		
		calculateParams(params_smart_mode, 0x59); 			// Y
		calculateParams(params_bye, 0x52);					// R
		EEPROM.begin(EEPROM_SIZE);
		workonbattery_count = EEPROM.readInt(address);
	}

	void loop() override {}

	void main_uart_read(uint8_t *command)
	{
		std::fill_n(Re_buf, 30, 0);
		write_array(command, 1);
		delay(100);
		while (available())
		{
			// Читение и запись данных из UART
			Re_buf[counter] = read();
			counter++;
		}
		//ESP_LOGD("apc_ups", "%d %d %d %d %d %d %d %d %d %d %d", Re_buf[0], Re_buf[1],Re_buf[2],Re_buf[3],Re_buf[4],Re_buf[5],Re_buf[6],Re_buf[7],Re_buf[8],Re_buf[9],Re_buf[10]);
		delay(100);
	}

	void update() override
	{

		std::fill_n(Re_buf, 30, 0);
//===============================================================================================
// ФОРМИРОВАНИЕ ЗАПРОСОВ
		ESP_LOGD("apc_ups", "step: %d", step);
		if (step == 0){
			main_uart_read(params_smart_mode);				// ПРИВЕТ
		}
		
		if (step == 1){
			main_uart_read(params_estimated_runtime);		// Оставшееся время
		}
		
		if (step == 2){
			main_uart_read(params_input_voltage);			// НАПРЯЖЕНИЕ вход
		}

		if (step == 3){
			main_uart_read(params_temperature);				// Температура
		}

		if (step == 4) {
			main_uart_read(params_battery_level);			// Заряд батареи в %
		}
		if (step == 5) {
			main_uart_read(params_power_load);				// Нагрузка
		}
		if (step == 6) {
			main_uart_read(params_output_voltage);			// НАПРЯЖЕНИЕ выход
		}
		if (step == 7) {
			main_uart_read(params_battery_voltage);			// НАПРЯЖЕНИЕ батареи
		}
		if (step == 8) {
			main_uart_read(params_line_frequency);			// частота
		}
		if (step == 9) {
			main_uart_read(params_status);					// Bit статус 
		}
		if (step == 10) {
			main_uart_read(params_last_cause);				// Последний случай отключения
		}

		if (step == 20) {
			main_uart_read(params_bye);						// ПОКА
		}

		counter = 0;
		

		error_cnt++;
		total_error_cnt++;
//==========================================================================================================
// ПРОВЕРКА ПОЛУЧЕНЫЙ ДАННЫХ
		if ((step == 0) && (Re_buf[0] == 0x53) && (Re_buf[1] == 0x4d) && (Re_buf[2] == 0x0d) && (Re_buf[3] == 0x0a))		
		{
			//ESP_LOGD("apc_ups", "Smart ON");
			std::fill_n(Re_buf, 30, 0);
			step = 1;
			error_cnt=0;
		}
		
	// estimated_runtime
		if ((step == 1) && (Re_buf[4] == 0x3a))		
		{
			double estimated_runtime = byteToFloat(Re_buf,4);
			std::fill_n(Re_buf, 30, 0);
			if (estimated_runtime > 0 && estimated_runtime < 100){
				if (Estimated_runtime != nullptr) Estimated_runtime->publish_state(estimated_runtime);
				error_cnt=0;
				step=2;
			};
		}
	//input_voltage
		if ((step == 2) && (Re_buf[5] == 0x0d) && (Re_buf[6] == 0x0a))		
		{
			double input_voltage = byteToFloat(Re_buf,5);
			//ESP_LOGD("apc_ups", "input_voltage=%d", input_voltage);
			
			std::fill_n(Re_buf, 30, 0);
			if ((input_voltage > 190 && input_voltage < 260) || (input_voltage==0.0) ){
				if (Input_voltage != nullptr) Input_voltage->publish_state(input_voltage);
				error_cnt=0;
				step=3;
			};
		}
	//temperature
		if ((step == 3) && (Re_buf[5] == 0x0d) && (Re_buf[6] == 0x0a))		
		{
			double temperature = byteToFloat(Re_buf,5);
			std::fill_n(Re_buf, 30, 0);
			if (temperature > 10 && temperature < 50){
	
				if (Temperature != nullptr) Temperature->publish_state(temperature);
				error_cnt=0;
				step=4;
			};
		}
	//battery_level
		if ((step == 4) && (Re_buf[5] == 0x0d) && (Re_buf[6] == 0x0a))	
		{
			double battery_level = byteToFloat(Re_buf,5);
			std::fill_n(Re_buf, 30, 0);
			if (battery_level > 1 && battery_level < 101){
				if (Battery_level != nullptr) Battery_level->publish_state(battery_level);
				error_cnt=0;
				step=5;
			};
		}
		
	//power_load
		if ((step == 5) && (Re_buf[5] == 0x0d) && (Re_buf[6] == 0x0a))	
		{
			double power_load = byteToFloat(Re_buf,5);
			std::fill_n(Re_buf, 30, 0);
			if (power_load > 1 && power_load < 101){
				if (Power_load != nullptr) Power_load->publish_state(power_load);
				error_cnt=0;
				step=6;
			};
		}
	//output_voltage
		if ((step == 6) && (Re_buf[3] == 0x0d) && (Re_buf[4] == 0x0a))	
		{
			double output_voltage = byteToFloat(Re_buf,3);
			std::fill_n(Re_buf, 30, 0);
			if (output_voltage > 190 && output_voltage < 260){
				if (Output_voltage != nullptr) Output_voltage->publish_state(output_voltage);
				error_cnt=0;
				step=7; 
			};
		}		
	//battery_voltage
		if ((step == 7) && (Re_buf[5] == 0x0d) && (Re_buf[6] == 0x0a))	
		{
			double battery_voltage = byteToFloat(Re_buf,5);
			std::fill_n(Re_buf, 30, 0);
			if (battery_voltage > 0 && battery_voltage < 30){
				if (Battery_voltage != nullptr) Battery_voltage->publish_state(battery_voltage);
				error_cnt=0;
				step=8; 
			};
		}			
	//line_frequency
		if ((step == 8) && (Re_buf[5] == 0x0d) && (Re_buf[6] == 0x0a))	
		{
			double line_frequency = byteToFloat(Re_buf,5);
			std::fill_n(Re_buf, 30, 0);
			if (line_frequency > 40 && line_frequency < 60){
				if (Line_frequency != nullptr) Line_frequency->publish_state(line_frequency);
				error_cnt=0;
				step=9; 
			};
		}			
	//status bits
		if ((step == 9) && (Re_buf[2] == 0x0d) && (Re_buf[3] == 0x0a))	
		{
			int status = byteToInt(Re_buf, 2);
			
			std::fill_n(Re_buf, 30, 0);
			//uint8_t read_status = 
			bool runtime_calibration = bitRead(status, 0);
			bool smart_trim = bitRead(status, 4);  // ИЗМЕНЕНО
			bool smart_boost = bitRead(status, 2);
			bool on_line = bitRead(status, 3);
			bool on_battery = bitRead(status, 1); // ИЗМЕНЕНО
			wrk_on_btr = on_battery;
			bool overloaded_output = bitRead(status, 5);
			bool battery_low = bitRead(status, 6);
			bool replace_battery = bitRead(status, 7);
			
			if (status > 0){
				if (Runtime_calibration != nullptr) Runtime_calibration->publish_state(runtime_calibration);
				if (Smart_trim != nullptr) Smart_trim->publish_state(smart_trim);
				if (Smart_boost != nullptr) Smart_boost->publish_state(smart_boost);
				if (On_line != nullptr) On_line->publish_state(on_line);
				if (On_battery != nullptr) On_battery->publish_state(on_battery);
				if (Overloaded_output != nullptr) Overloaded_output->publish_state(overloaded_output);
				if (Battery_low != nullptr) Battery_low->publish_state(battery_low);
				if (Replace_battery != nullptr) Replace_battery->publish_state(replace_battery);
				error_cnt=0;
				step=10; 
			};
		}		
		
	//last_cause
		if ((step == 10) && (Re_buf[1] == 0x0d) && (Re_buf[2] == 0x0a))	
		{
			uint8_t temp_cause = Re_buf[0];
			String last_cause = "";
			std::fill_n(Re_buf, 30, 0);
			switch(temp_cause) {
				case 0x52: last_cause = "Unacceptable utility voltage rate of change";
				break;
				case 0x48: last_cause = "High utility voltage";
				break;
				case 0x4c: last_cause = "low utility voltage";
				break;
				case 0x54: last_cause = "line voltage notch or spike";
				break;
				case 0x4f: last_cause = "no transfers since turnon";
				break;
				case 0x53: last_cause = "transfer due to U command or activation of UPS test from front panel";
				break;
			}
			
			if (last_cause != ""){
				if (Last_cause != nullptr) Last_cause->publish_state(last_cause.c_str());
				error_cnt=0;
				total_error_cnt=0;
				status = "ONLINE";
				step=1; 
			};
		}	
		
		if (error_cnt > 5){
			error_cnt=0;
			step=0;
		}
		if (total_error_cnt > 2000){
			total_error_cnt=1000;
		}

		if (total_error_cnt > 60){
			status = "OFFLINE";
		}	
		// СЧЕТЧИК РАБОТЫ НА БАТАРЕЕ
		if (status == "ONLINE") {
			if (wrk_on_btr == true){
				status = "ONBATT";
				if (wrk_on_btr_trig==false){
					wrk_on_btr_trig = true;
					workonbattery_count++;
					EEPROM.writeInt(address, workonbattery_count);
					EEPROM.commit ();
					int32_t check_eeprom_read = EEPROM.readInt(address);
					ESP_LOGD("apc_ups", ">>>>>>>EEPROM READ DATA =%d", check_eeprom_read);

				}
			}
			else{
				wrk_on_btr_trig=false;
			}
			if (WorkOnBattery_count != nullptr) WorkOnBattery_count->publish_state(workonbattery_count);
		}
		
		//ESP_LOGD("apc_ups", "step=%d, pv_step=%d, count=%d, tcount=%d, status=%s", step, prev_step,error_cnt,total_error_cnt,status);
//		ESP_LOGD("apc_ups", "wrk_on_btr=%d, wrk_on_btr_trig=%d, workonbattery_count=%d", wrk_on_btr, wrk_on_btr_trig,workonbattery_count);

	
		if (status != ""){
			if (Status != nullptr) Status->publish_state(status.c_str());
		}
		
		if ((step == 20) && (Re_buf[0] == 0x42) && (Re_buf[1] == 0x59) && (Re_buf[2] == 0x45) && (Re_buf[3] == 0x0d) && (Re_buf[4] == 0x0a))
		{
			ESP_LOGD("apc_ups", "Bye bye OK");
			std::fill_n(Re_buf, 30, 0);
			step=0;
		}

	};
};