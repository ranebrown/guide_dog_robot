
# include "i2cfun.h"

int i2c_set_speed (uint32_t speed,uint32_t pba_hz)
{
	uint32_t f_prescaled;
	uint8_t cwgr_exp = 0;
	f_prescaled = (pba_hz / speed / 2);
	// f_prescaled must fit in 8 bits, cwgr_exp must fit in 3 bits
	while ((f_prescaled > 0xFF) && (cwgr_exp <= 0x7)) {
		// increase clock divider
		cwgr_exp++;
		// divide f_prescaled value
		f_prescaled /= 2;
	}
	if (cwgr_exp > 0x7) {
		return 0;
	}
	// set clock waveform generator register
	*(volatile uint32_t *)0xFFFF4404 = ((f_prescaled/2))
	| ((f_prescaled - f_prescaled/2) << 8)
	| (cwgr_exp << 28)
	| (0     << 24)
	| (f_prescaled << 16);
	return 1;
}

void i2c_init(void){
	i2c_set_speed(100000,17200000);
	*(volatile uint32_t *)0xFFFF4424 = 0x0000ffff;					//disable interupts
	*(volatile uint32_t *)0xFFFF4400 = 0x00000021;					//enable master interface
}

int i2c_write(int addr,char * data,int numbytes){
	int i;
	*(volatile uint32_t *)0xFFFF440C = 0x0000E000 | ( (0x7F & addr) << 1 ) | ((0xFF & numbytes)<<16);		//load command reg
	for (i = 0; i < numbytes; i++){
		while(!(*((volatile uint32_t*)(0xFFFF441C)) & 2)){									//wait for tx reg to empty
			if (*((volatile uint32_t*)(0xFFFF441C)) & 0x00000300){							//if no ack exit
				*((volatile uint32_t*)(0xFFFF442C)) = 0x00000300;
				return 0;
			}
		}
		*(volatile uint32_t *)0xFFFF4418 = *(volatile char *)data++;											//load data reg
	}
	return 1;
}

int i2c_read(int addr,int numbytes){
	char data[numbytes];
	int i;
	int temp = 0;
	*(volatile uint32_t *)0xFFFF440C = 0x0000E000 | ( (0x7F & addr) << 1 ) | ((0xFF & numbytes)<<16) | 1;
	for (i = 0; i < numbytes; i++){
		while(!(*((volatile uint32_t*)(0xFFFF441C)) & 1)){
			if (*((volatile uint32_t*)(0xFFFF441C)) & 0x00000300){
				*((volatile uint32_t*)(0xFFFF442C)) = 0x00000300;
				return 0;
			}
		}
		data[i] = *(volatile uint32_t *)0xFFFF4414;
		temp = temp<<8 | data[i];
	}
	return temp;
}

int getLidar(void){
	int val = -1;
	char data1[2] = {0x00,0x04};
	char data2 = 0x8f;
	i2c_write(0x62,&data1,2);
	val = val*2;
	i2c_write(0x62,&data2,1);
	val = val*2;
	val = i2c_read(0x62,2);
	return val;
}
