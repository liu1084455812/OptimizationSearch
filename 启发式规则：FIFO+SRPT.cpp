#include <iostream>
#include <fstream>
#include <ctime>
#include <windows.h>
using namespace std;
const int test_times_max = 50;	//���Դ�������

class MACHINE//��¼�û����Ƿ����ڹ���������¼��ʣʱ��
{
public:
	int used;
	int need_time;
	int next_job;
	MACHINE(int u = 0, int t = 0, int nj = -1) :used(u), need_time(t), next_job(nj){};
	void machine_init(){
		used = 0;
		need_time = 0;
		next_job = -1;
	}

};
class JOB{
public:
	int used;
	int need_time;
	int now_setp;
	JOB(int u = 0, int t = 0, int ns = 1) :used(u), need_time(t), now_setp(ns){};
	void job_init(){
		used = 0;
		need_time = 0;
		now_setp = 1;
	}
};
int job_number, machine_number, setp_number;
int machine_of_job_setp[1010][1010], time_of_job_setp[1010][1010];//job_number,setp_number
MACHINE machine_anrry[1010];//��¼�û����Ƿ����ڹ���������¼��ʣʱ��
JOB job_anrry[1010];//��¼�ù����Ƿ����ڹ���������¼��ʣʱ��

int main(){
	int the_best = INT_MAX;
	int the_worst = 0;
	int the_global_sum_time = 0;
	int test_times = 1;
	//time_t t1 = time(0);
	SYSTEMTIME  sys1;
	GetLocalTime(&sys1);
	ofstream f2("out6.txt");
	for (test_times = 1; test_times <= test_times_max; test_times++){
		ifstream f1;
		f1.open("in6.txt");
		char c;
		f1 >> job_number >> machine_number;// >> c>> c>> setp_number >> c
		setp_number = machine_number;
		for (int i = 1; i <= job_number; i++){
			for (int j = 1; j <= setp_number; j++){
				f1 >> machine_of_job_setp[i][j] >> time_of_job_setp[i][j];
				machine_of_job_setp[i][j]++;
			}
		}
		for (int i = 1; i <= job_number; i++){
			job_anrry[i].job_init();
		}
		for (int i = 1; i <= machine_number; i++){
			machine_anrry[i].machine_init();
		}
		//for (int i = 1; i <= job_number; i++){
		//	for (int j = 1; j <= setp_number; j++){
		//		f1 >> machine_of_job_setp[i][j];
		//	}
		//}

		int temp_min_time = INT_MAX, min_time = INT_MAX, sum_time = 0;
		while (1){
			int complete = 0;//�ж��Ƿ����еĹ�������ɼӹ�
			for (int i = 1; i <= job_number; ++i){
				if (job_anrry[i].now_setp>setp_number)complete++;
			}

			if (complete == job_number)break;//���ȫ���������ӹ�����˳�

			for (int i = 1; i <= machine_number; i++){
				if (machine_anrry[i].used == 0){
					temp_min_time = INT_MAX;
					// int fing_flag=0;//�ж��ҵ���һ������
					for (int j = 1; j <= job_number; j++){//�ҵ�ǰ��������һ���ӹ�����
						if (job_anrry[j].used == 0 && \
							job_anrry[j].now_setp <= setp_number && \
							machine_of_job_setp[j][job_anrry[j].now_setp] == i && \
							time_of_job_setp[j][job_anrry[j].now_setp]<temp_min_time
							){
							machine_anrry[i].next_job = j;
							temp_min_time = time_of_job_setp[j][job_anrry[j].now_setp];
						}
					}
					if (temp_min_time != INT_MAX){
						int next_job = machine_anrry[i].next_job;;
						machine_anrry[i].used = 1;
						machine_anrry[i].need_time = time_of_job_setp[next_job][job_anrry[next_job].now_setp];
						job_anrry[next_job].used = 1;
						job_anrry[next_job].need_time = machine_anrry[i].need_time;
						//job_anrry[next_job].now_setp++;

						//f2 << next_job << " ";
					}
				}
			}

			//�ҵ���Сʱ��
			min_time = INT_MAX;
			for (int i = 1; i <= machine_number; i++){
				if (machine_anrry[i].used == 1 && \
					machine_anrry[i].need_time<min_time)
					min_time = machine_anrry[i].need_time;
			}
			sum_time += min_time;
			//ʹ��ʱ��
			for (int i = 1; i <= machine_number; i++){
				if (machine_anrry[i].used == 1){
					machine_anrry[i].need_time -= min_time;
					if (machine_anrry[i].need_time <= 0){
						machine_anrry[i].need_time = 0;
						machine_anrry[i].next_job = -1;
						machine_anrry[i].used = 0;
					}
				}
			}
			//
			for (int i = 1; i <= job_number; i++){
				if (job_anrry[i].used == 1){
					job_anrry[i].need_time -= min_time;
					if (job_anrry[i].need_time <= 0){
						job_anrry[i].used = 0;
						job_anrry[i].need_time = 0;
						job_anrry[i].now_setp++;
					}
				}
			}
			int debug = 1;
		}
		f2 << "��FIFO+SRPT����ʽ����õ��Ľ�: " << sum_time << endl;

		the_global_sum_time += sum_time;
		if (sum_time<the_best)the_best = sum_time;
		if (sum_time>the_worst)the_worst = sum_time;
		f1.close();
	}
	SYSTEMTIME sys2;
	GetLocalTime(&sys2);
	//time_t t2 = time(0);
	cout << "�ļ�����ʱ��: " << sys1.wMinute << sys1.wMilliseconds << endl;
	cout << "�ļ��ر�ʱ��: " << sys2.wMinute << sys2.wMilliseconds << endl;
	f2 << "��ý�: " << the_best << endl;
	f2 << "���: " << the_worst << endl;
	f2 << "ƽ����: " << the_global_sum_time / test_times_max << endl;
	cout << "��ý�: " << the_best << endl;
	cout << "���: " << the_worst << endl;
	cout << "ƽ����: " << the_global_sum_time / test_times_max << endl;
	//f2 << "ƽ��ʱ��: " << ( 60*double(sys2.wHour-sys1.wHour)+(double(sys2.wMinute-sys1.wMinute)+(double(sys2.wMilliseconds - sys1.wMilliseconds)/1000000)))/ test_times_max << endl;
	f2 << "ƽ��ʱ��: " << (60 * double(sys2.wMinute - sys1.wMinute) + (double(sys2.wSecond - sys1.wSecond) + (double(sys2.wMilliseconds - sys1.wMilliseconds) / 1000000))) / test_times_max << endl;
	cout << "ƽ��ʱ��: " << ((60 * double(sys2.wMinute - sys1.wMinute)+double(sys2.wSecond - sys1.wSecond) + (double(sys2.wMilliseconds - sys1.wMilliseconds) / 1000000))) / test_times_max << endl;
	f2.close();
	return 0;
}