//UPS APC ESPhome
//My telegram https://t.me/DieMetRik

#include "esphome.h"
#include "esphome/core/log.h"
#include "EEPROM.h"
#define EEPROM_SIZE 8

//namespace esphome {
//namespace mercury {

bool sw_selftest = false;
bool run_calibration_sw = false;

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
	Sensor *Self_test_interval {nullptr};
	TextSensor *St_result {nullptr};

	Sensor *Return_threshold {nullptr};
	Sensor *Grace_delay {nullptr};
	Sensor *Wakeup_delay {nullptr};
	TextSensor *Sensitivity {nullptr};
	BinarySensor *Self_test_progress {nullptr};
	
	
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
	Sensor *sensor19,
	Sensor *sensor20,
	TextSensor *sensor21,
	Sensor *sensor22,
	Sensor *sensor23,
	Sensor *sensor24,
	TextSensor *sensor25,
	BinarySensor *sensor26
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
	WorkOnBattery_count(sensor19),
	Self_test_interval(sensor20),
	St_result(sensor21),
	Return_threshold(sensor22),
	Grace_delay(sensor23),
	Wakeup_delay(sensor24),
	Sensitivity(sensor25),
	Self_test_progress(sensor26)
	{}
	
	//int sw_Self_test;
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
	unsigned char params_output_voltage[1];		// O = 4f
	unsigned char params_battery_voltage[1]; 	// B
	unsigned char params_estimated_runtime[1]; 	// j
	unsigned char params_line_frequency[1]; 	// F 
//Text sensor
	unsigned char params_last_cause[1]; 		// G
	unsigned char params_self_test_interval[1]; // E
	unsigned char params_self_test_run[1]; 		// W = 57
	unsigned char params_self_test_read[1];		// X = 58
	
	unsigned char params_return_threshold[1]; 	// e = 65
	unsigned char params_grace_delay[1]; 		// p = 70
	unsigned char params_wakeup_delay[1]; 		// r = 72
	unsigned char params_sensitivity[1]; 		// s = 73
	unsigned char params_run_calibration[1];	// D = 44	
	
	uint8_t Re_buf[40];
	int counter = 0;

	String status = "OFFLINE";
	
	int error_cnt = 0;
	int st_cnt = 0; //self test counter
	int total_error_cnt = 0;
	int step = 0;
	int prev_step = 0;

	int interval = 1000; 		// ИНТЕРВАЛ ОБНОВЛЕНИЯ
	bool b_self_test_progress = false;	//self test in progress
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
	int string2hex(char *s, uint8_t cnt){
		int x = 0;
		for(int i = 0;cnt;i++) {
			char c = *s;
			if (c >= '0' && c <= '9') {
				x *= 16;
				x += c - '0'; 
			}
			else if (c >= 'A' && c <= 'F') {
				x *= 16;
				x += (c - 'A') + 10; 
			}
			else break;
				s++;
		}
		return x;
	}
	
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
		calculateParams(params_output_voltage, 0x4f);		// O
		calculateParams(params_battery_voltage, 0x42);		// B
		calculateParams(params_line_frequency, 0x46); 		// F 

		calculateParams(params_status, 0x51);				// Q
		calculateParams(params_last_cause, 0x47); 			// G
		
		calculateParams(params_smart_mode, 0x59); 			// Y
		calculateParams(params_bye, 0x52);					// R
		calculateParams(params_self_test_interval, 0x45);	// E
		calculateParams(params_self_test_run, 0x57);		// W = 57
		calculateParams(params_self_test_read, 0x58);		// X = 58

		calculateParams(params_return_threshold, 0x65);		// e = 65
		calculateParams(params_grace_delay, 0x70);			// p = 70
		calculateParams(params_wakeup_delay, 0x72);			// r = 72
		calculateParams(params_sensitivity, 0x73);			// s = 73

		calculateParams(params_run_calibration, 0x44);		// D = 44	



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
		if (step == 11) {
			main_uart_read(params_self_test_interval);		// Время самотестирования
		}

		if (step == 12) {
			main_uart_read(params_return_threshold);		// Время возвращения
		}
		if (step == 13) {
			main_uart_read(params_grace_delay);				// Время в секундах ухода в soft shutdown
		}
		if (step == 14) {
			main_uart_read(params_wakeup_delay);			// wakeup_delay
		}
		if (step == 15) {
			main_uart_read(params_sensitivity);				// sensitivity
		}
		
//**************************** SELF test section*********************************************		
		if (step == 30) {
			main_uart_read(params_self_test_run);			// Запуск самотестирования
			step = 31;
		}
		if (step == 32) {
			if ((st_cnt > 60) && b_self_test_progress) {
				main_uart_read(params_self_test_read);			// Запуск самотестирования
			}
			else{
			step = 1;
			}
				
		}
		if (b_self_test_progress){
			st_cnt++;
			ESP_LOGD("apc_ups", "%d", st_cnt);
		}
		if ((st_cnt > 200) && b_self_test_progress) {
			b_self_test_progress = false;
			st_cnt = 0;
		}

		
//**************************** END SELF test section*********************************************		
		if (step == 40) {
			main_uart_read(params_run_calibration);			// Запуск самотестирования
			step = 41;
		}
//**************************** END CALIBRATION*********************************************		

		
		if (step == 20) {
			main_uart_read(params_bye);						// ПОКА
		}
		//ESP_LOGD("apc_ups", "%d %d %d", params_self_test_interval_set[0], params_self_test_interval_set[1],params_self_test_interval_set[2]);
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
			if (temperature > 2 && temperature < 50){
	
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
		if ((step == 6) && (Re_buf[5] == 0x0d) && (Re_buf[6] == 0x0a))	
		{
			double output_voltage = byteToFloat(Re_buf,5);
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
			if (battery_voltage > 0 && battery_voltage < 100){
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
			int status = string2hex((char*)Re_buf, 1);
			std::fill_n(Re_buf, 30, 0);
			//uint8_t read_status = 
			bool runtime_calibration = bitRead(status, 0); 
			bool smart_trim = bitRead(status, 1);  
			bool smart_boost = bitRead(status, 2);
			bool on_line = bitRead(status, 3);
			bool on_battery = bitRead(status, 4); 
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
				step=11; 
			};
		}	
		//params_self_test_interval
		if ((step == 11) && (Re_buf[3] == 0x0d) && (Re_buf[4] == 0x0a))	
		{
			double self_test_interval = -1;
			if (Re_buf[0] == 0x4f) {
				self_test_interval = 0;
			}
			else
			{
				self_test_interval = byteToInt(Re_buf,3);
			}
			
			std::fill_n(Re_buf, 30, 0);
			if (self_test_interval >= 0 && self_test_interval < 1000){
				if (Self_test_interval != nullptr) Self_test_interval->publish_state(self_test_interval);
				step = 12;
				error_cnt=0;
			};
		}	
		
	//return_threshold
		if ((step == 12) && (Re_buf[2] == 0x0d) && (Re_buf[3] == 0x0a))	
		{
			int return_threshold = byteToInt(Re_buf,2);
			std::fill_n(Re_buf, 30, 0);
			if (return_threshold >= 0 && return_threshold < 1000){
				if (Return_threshold != nullptr) Return_threshold->publish_state(return_threshold);
				error_cnt=0;
				step=13; 
			};
		}				
		
	//grace_delay
		if ((step == 13) && (Re_buf[3] == 0x0d) && (Re_buf[4] == 0x0a))	
		{
			int grace_delay = byteToInt(Re_buf,3);
			std::fill_n(Re_buf, 30, 0);
			if (grace_delay >= 0 && grace_delay < 1000){
				if (Grace_delay != nullptr) Grace_delay->publish_state(grace_delay);
				error_cnt=0;
				step=14; 
			};
		}			
	//wakeup_delay
		if ((step == 14) && (Re_buf[3] == 0x0d) && (Re_buf[4] == 0x0a))	
		{
			int wakeup_delay = byteToInt(Re_buf,3);
			std::fill_n(Re_buf, 30, 0);
			if (wakeup_delay >= 0 && wakeup_delay < 1000){
				if (Wakeup_delay != nullptr) Wakeup_delay->publish_state(wakeup_delay);
				error_cnt=0;
				step=15; 
			};
		}					
	//sensitivity
		if ((step == 15) && (Re_buf[1] == 0x0d) && (Re_buf[2] == 0x0a))	
		{
			uint8_t temp_cause = Re_buf[0];
			String sensitivity = "";
			std::fill_n(Re_buf, 30, 0);
			switch(temp_cause) {
				case 0x48: sensitivity = "Highest";
				break;
				case 0x4d: sensitivity = "Medium";
				break;
				case 0x4c: sensitivity = "Lowest";
				break;
				case 0x41: sensitivity = "Autoadjust";
				break;
			}
			if (sensitivity != ""){
				if (Sensitivity != nullptr) Sensitivity->publish_state(sensitivity.c_str());
				total_error_cnt=0;
				status = "ONLINE";
				error_cnt=0;
				if (b_self_test_progress){
					step=32; 
				}
				else{
					step=1; 
				}
			};
		}				
//******************************END MAIN SENSORS**************************************************************		


//****************************** SELF TEST**************************************************************		

		if (sw_selftest){ //request self test step
			step=30; 
			error_cnt=0;
			sw_selftest=false;
		}
		
		if ((step == 31) && (Re_buf[2] == 0x0d) && (Re_buf[3] == 0x0a))	{ //wait OK from start self sheck
			if ((Re_buf[0] == 0x4f) && (Re_buf[1] == 0x4b)) {
				std::fill_n(Re_buf, 30, 0);
				error_cnt=0;
				b_self_test_progress = true;
				step = 1;
			}
		}
		if ((step == 32) && (Re_buf[2] == 0x0d) && (Re_buf[3] == 0x0a))	{ //wait 60 sec after selt test
			String st_result = "";
			
			if ((Re_buf[0] == 0x4f) && (Re_buf[1] == 0x4b)) {
				st_result = "OK";
				b_self_test_progress = false;
				st_cnt = 0;
				step = 1;
			}
			if ((Re_buf[0] == 0x42) && (Re_buf[1] == 0x54)) {
				st_result = "Battery Fault";
				b_self_test_progress = false;
				st_cnt = 0;
				step = 1;
			}
			if ((Re_buf[0] == 0x4e) && (Re_buf[1] == 0x47)) {
				st_result = "Overload";
				b_self_test_progress = false;
				st_cnt = 0;
				step = 1;
			}
			if ((Re_buf[0] == 0x4e) && (Re_buf[1] == 0x4f)) {
				st_result = "No Result";
			}
			if (St_result != nullptr) St_result->publish_state(st_result.c_str());
			std::fill_n(Re_buf, 30, 0);
			error_cnt=0;
		}
		if (Self_test_progress != nullptr) Self_test_progress->publish_state(b_self_test_progress);
//******************************END SELF TEST**************************************************************		

		
//****************************** CALIBRATION**************************************************************		
		if (run_calibration_sw){ //request self test step
			step=40; 
			error_cnt=0;
			run_calibration_sw=false;
		}
		
		if ((step == 41) && (Re_buf[1] == 0x0d) && (Re_buf[2] == 0x0a))	{ //wait OK from run calibration
			if (Re_buf[0] == 0x21) {
				std::fill_n(Re_buf, 30, 0);
				error_cnt=0;
				step = 1;
			}
		}
		
//****************************** END CALIBRATION**************************************************************		

			
					
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

class SelfTestSwitch : public Component, public Switch {
	public:
	void setup() override {
	}
	
	void write_state(bool state) override {
		sw_selftest = state;
		publish_state(state);
	};

	
};

class Run_CalibrationSwitch : public Component, public Switch {
	public:
	void setup() override {
	}
	
	void write_state(bool state) override {
		run_calibration_sw = state;
		publish_state(state);
	};

	
};

