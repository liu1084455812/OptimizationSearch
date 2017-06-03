//杂交后老的染色体要留下
#include <iostream>
#include <cmath>
#include <fstream>
#include <ctime>
#include <windows.h>
#include <queue>
#include <vector>
using namespace std;
/*
*0: SPT:最短加工时间优先（已加工时间最短）
*1: SRPT:最短加工时间优先（下一个操作加工用时最短）
*2: SRT:最短剩余时间优先（计算最短剩余时间）
*3：TIS:最长等待时间优先(当前用时减去在工件上加工的用时)
*/
//参数
const int test_times_max = 50;				//测试次数上限
const int no_change_max_times = 50;			//不变上限次数
const int debug = 0;						//DEBUG
const int heuristic_number_max = 3;			//启发式规则数上限
const int times_max = 300;					//迭代次数上限
const int chromosome_number_max = 50;		//染色体数上限(最多可同时存在chromosome_number_max*2个染色体)
const double PI = 3.1415926;				//PI
const double k1 = 1.0;						//F>F_AVG时的最高交叉概率
const double k2 = 1.0;						//F<F_AVG时的最高交叉概率
const double k3 = 0.5;						//F>F_AVG时的最高变异概率
const double k4 = 0.5;						//F<F_AVG时的最高变异概率
const int variation_number_max = 3;			//变异基因的上限个数
//
int job_number, machine_number, setp_number;
//
class MACHINE{
public:
	int next_job;
	int used;
	int need_time;
	static int machine_number;
	MACHINE(int u = 0, int t = 0, int nj = -1) :next_job(nj), used(u), need_time(t){};
	void machine_init(){ next_job = -1; used = 0; need_time = 0; }
};
int MACHINE::machine_number = 0;
class JOB{
public:
	int now_setp;
	int used;
	int need_time;
	int used_time;
	int rest_time;
	int begin_wait;
	static int job_number;
	static int setp_number;
	JOB(int u = 0, int t = 0, int NS = 1, int ut = 0, int rt = INT_MAX, int bw = 0) :begin_wait(bw), now_setp(NS), used(u), need_time(t), used_time(ut), rest_time(rt){};
	void job_init(){ begin_wait = 0; now_setp = 1; used = 0;	need_time = 0; used_time = 0; rest_time = INT_MAX; };
};
int JOB::job_number = 0;
int JOB::setp_number = 0;

int machine_of_job_setp[1010][1010], time_of_job_setp[1010][1010];//job_number,setp_number
MACHINE machine_anrry[1010];//记录该机器是否正在工作，并记录所剩时间
JOB job_anrry[1010];//记录该工件是否正在工作，并记录所剩时间

class chromosome{
public:
	int size;	//染色体信息
	int *DNA;
	int using_time;
	double adaptation;
	double PC, PM;
	static int max_using_time;
	static double avg_adaptation;
	static double max_adaptation;
	chromosome() :size(machine_number), using_time(INT_MAX){
		DNA = new int[size + 1];
		for (int i = 1; i <= size; ++i){
			DNA[i] = rand() % heuristic_number_max;
		}
	};
	chromosome(const chromosome& temp){
		size = temp.size;
		using_time = temp.using_time;
		adaptation = temp.adaptation;
		PC = temp.PC;
		PM = temp.PM;
		DNA = new int[size + 1];
		for (int i = 1; i <= size; i++){
			DNA[i] = temp.DNA[i];
		}
	}
	friend bool operator <(const chromosome& temp1, const chromosome& temp2){
		return temp1.adaptation < temp2.adaptation;
	};
	bool operator == (const chromosome& temp){
		return DNA == temp.DNA;
	};
	chromosome& operator = (const chromosome& temp){
		if (this == &temp) return *this;
		size = temp.size;
		using_time = temp.using_time;
		adaptation = temp.adaptation;
		PC = temp.PC;
		PM = temp.PM;
		delete[] DNA;
		DNA = new int[size + 1];
		for (int i = 1; i <= size; i++){
			DNA[i] = temp.DNA[i];
		}
		return *this;
	}

	void cross(chromosome& temp){//杂交产生新的染色体 
		int the_number_dna_change = 1 + rand() % (size / 2);
		int the_begin_of_change = 1 + rand() % size;
		the_number_dna_change = (size - the_begin_of_change + 1)<the_number_dna_change ? \
			(size - the_begin_of_change + 1) : the_number_dna_change;
		for (int i = the_begin_of_change; i< the_begin_of_change + the_number_dna_change; ++i){
			int temp_int = DNA[i];
			DNA[i] = temp.DNA[i];
			temp.DNA[i] = temp_int;
		}
	}
	void variation(){
		for (int i = 1; i <= size; i++){
			if (rand() / double(RAND_MAX) < PM){
				DNA[i] = rand() % heuristic_number_max;
			}
		}
	}

	void refresh(){//引用？可以数组方式访问下一个吗？
		int job_number = JOB::job_number;
		int machine_number = MACHINE::machine_number;
		int setp_number = JOB::setp_number;
		for (int i = 1; i <= job_number; i++)	job_anrry[i].job_init();
		for (int i = 1; i <= machine_number; i++)	machine_anrry[i].machine_init();

		int min_time = INT_MAX, sum_time = 0;
		//初始化
		while (1){
			int complete = 0;//判断是否所有的工件都完成加工
			for (int i = 1; i <= job_number; ++i){
				if (job_anrry[i].now_setp>setp_number)complete++;
			}
			if (complete == job_number)break;//如果全部工件都加工完就退出
			//找到每一机器下一个要加工的工件
			for (int i = 1; i <= machine_number; i++){
				if (machine_anrry[i].used == 0){
					switch (DNA[i]){
					case 0: SPT(machine_anrry[i], job_anrry, i); break;
					case 1: SRPT(machine_anrry[i], job_anrry, i); break;
					case 2: TIS(machine_anrry[i], job_anrry, i, sum_time); break;
						//	case 3:	SRT(machine_anrry[i],job_anrry,i);
					}
				}
			}

			//找到最小时间
			min_time = INT_MAX;
			for (int i = 1; i <= machine_number; i++){
				if (machine_anrry[i].used == 1 && \
					machine_anrry[i].need_time<min_time)
					min_time = machine_anrry[i].need_time;
			}

			sum_time += min_time;
			//使用时间
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
				job_anrry[i].used_time += min_time;
				if (job_anrry[i].used == 1){
					job_anrry[i].need_time -= min_time;
					if (job_anrry[i].need_time <= 0){
						//job_anrry[i].rest_time=0
						job_anrry[i].begin_wait = sum_time;
						job_anrry[i].used = 0;
						job_anrry[i].need_time = 0;
						job_anrry[i].now_setp++;
					}
				}
			}
			int debug = 0;
		}
		if (sum_time>chromosome::max_using_time)chromosome::max_using_time = sum_time;
		using_time = sum_time;
		/*adaptation = 1 - double(using_time) / chromosome::max_using_time;*/
	}

	void SPT(MACHINE& machine, JOB* job_anrry, int now_machine_number){
		int job_number = JOB::job_number;
		int setp_number = JOB::setp_number;
		int the_shortest_time = INT_MAX;
		for (int j = 1; j <= job_number; j++){
			if (job_anrry[j].used == 0 && \
				job_anrry[j].now_setp <= setp_number&& \
				machine_of_job_setp[j][job_anrry[j].now_setp] == now_machine_number&&\
				job_anrry[j].used_time<the_shortest_time
				){
				machine.next_job = j;
				the_shortest_time = job_anrry[j].used_time;
			}
		}
		if (the_shortest_time != INT_MAX){
			int next_job = machine.next_job;
			machine.used = 1;
			machine.need_time = time_of_job_setp[next_job][job_anrry[next_job].now_setp];
			job_anrry[next_job].used = 1;
			job_anrry[next_job].need_time = machine.need_time;
		}
	}

	void SRPT(MACHINE& machine, JOB* job_anrry, int now_machine_number){
		int job_number = JOB::job_number;
		int setp_number = JOB::setp_number;
		int the_next_shortest_time = INT_MAX;
		for (int j = 1; j <= job_number; j++){
			if (job_anrry[j].used == 0 && \
				job_anrry[j].now_setp <= setp_number&& \
				machine_of_job_setp[j][job_anrry[j].now_setp] == now_machine_number&&\
				time_of_job_setp[j][job_anrry[j].now_setp] < the_next_shortest_time
				){
				machine.next_job = j;
				the_next_shortest_time = time_of_job_setp[j][job_anrry[j].now_setp];
			}
		}
		if (the_next_shortest_time != INT_MAX){
			int next_job = machine.next_job;
			machine.used = 1;
			machine.need_time = time_of_job_setp[next_job][job_anrry[next_job].now_setp];
			job_anrry[next_job].used = 1;
			job_anrry[next_job].need_time = machine.need_time;
		}
	}
	void TIS(MACHINE& machine, JOB* job_anrry, int now_machine_number, int sum_time){
		int job_number = JOB::job_number;
		int setp_number = JOB::setp_number;
		int the_max_waiting_time = -1;
		for (int j = 1; j <= job_number; j++){
			if (job_anrry[j].used == 0 && \
				job_anrry[j].now_setp <= setp_number&& \
				machine_of_job_setp[j][job_anrry[j].now_setp] == now_machine_number&&\
				sum_time - job_anrry[j].begin_wait > the_max_waiting_time
				){
				machine.next_job = j;
				the_max_waiting_time = sum_time - job_anrry[j].begin_wait;
			}
		}
		if (the_max_waiting_time != -1){
			int next_job = machine.next_job;
			machine.used = 1;
			machine.need_time = time_of_job_setp[next_job][job_anrry[next_job].now_setp];
			job_anrry[next_job].used = 1;
			job_anrry[next_job].need_time = machine.need_time;
		}
	}

};
double chromosome::avg_adaptation = 0;
double chromosome::max_adaptation = 0;
int chromosome::max_using_time = 0;




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
		srand((int)time(0));
		//cout << (int)time(0) << endl;



		ifstream f1;
		f1.open("in6.txt");
		char c;
		f1 >> job_number >> machine_number;// >> c>> c>> setp_number >> c
		setp_number = machine_number;


		JOB::job_number = job_number;
		MACHINE::machine_number = machine_number;
		JOB::setp_number = setp_number;
		chromosome BEST_CHROME;
		// time_of_job_setp = new int*[job_number + 1];
		// machine_of_job_setp = new int*[job_number + 1];
		// machine_anrry = new MACHINE[machine_number + 1];
		// job_anrry = new JOB[job_number + 1];
		// for (int i = 0; i <= job_number; ++i){
		// 	time_of_job_setp[i] = new int[setp_number + 1];
		// 	machine_of_job_setp[i] = new int[setp_number + 1];
		// }
		//读入数组
		for (int i = 1; i <= job_number; i++){
			for (int j = 1; j <= setp_number; j++){
				f1 >> machine_of_job_setp[i][j] >> time_of_job_setp[i][j];
				machine_of_job_setp[i][j]++;
			}
		}
		// for (int i = 1; i <= job_number; i++){
		// 	job_anrry[i].job_init();
		// }
		// for (int i = 1; i <= machine_number; i++){
		// 	machine_anrry[i].machine_init();
		// }
		// int i = 0;
		//读入数组 	END
		//初始化染色体
		std::vector<chromosome> anrry;
		for (int i = 1; i <= chromosome_number_max; ++i){
			chromosome temp;
			anrry.push_back(temp);
		}
		//初始化染色体完成
		//进行迭代操作
		int before_best = INT_MAX;
		int no_change_times = 0;
		int times = 1;
		for (times = 1; times <= times_max; ++times){//迭代第times次

			//刷新
			chromosome::max_using_time = 0;
			for (int i = 1; i <= anrry.size(); i++){
				anrry[i - 1].refresh();
				//if (debug == 1){
				//	if (anrry[i - 1].using_time < 0){
				//		cout << "tiems: " << times << "i-1: " << i - 1 << endl << "DNA:";
				//		for (int j = 1; j <= machine_number; j++){
				//			cout << anrry[i - 1].DNA[j] << " ";
				//		}
				//		cout << endl;
				//		goto A;
				//	}

				//}
			}
			for (int i = 1; i <= anrry.size(); i++){
				anrry[i - 1].adaptation = 1 - double(anrry[i - 1].using_time) / chromosome::max_using_time;
				if (BEST_CHROME.using_time > anrry[i - 1].using_time) BEST_CHROME = anrry[i - 1];
				//if (debug == 1){
				//	if (anrry[i - 1].using_time < 0){
				//		cout << "error"<<endl;
				//		cout << "tiems: " << times << " i-1: " << i - 1 << endl << " DNA:";
				//		for (int j = 1; j <= machine_number; j++){
				//			cout << anrry[i - 1].DNA[j] << " ";
				//		}
				//		cout << endl;
				//		goto A;
				//	}

				//}
			}
			//刷新 END

			//判断是否有的best 如果多次未更新则退出
			if (before_best == BEST_CHROME.using_time) no_change_times++;
			else {
				before_best = BEST_CHROME.using_time;
				no_change_times = 0;
			}
			if (no_change_times >= no_change_max_times){
				break;
			}
			//判断是否有的best END

			//选择chromosome_number_max个染色体进入待选池子(选择适应度高的进入池子)
			priority_queue<chromosome>	Queue;
			std::vector<chromosome> temp_anrry;
			for (int i = 1; i <= anrry.size(); i++){
				Queue.push(anrry.at(i - 1));
			}
			anrry.clear();//太老的死亡
			double sum_adaptation = 0, max_adaptation = 0, avg_adaptation = 0;
			for (int i = 1; i <= chromosome_number_max; i++){
				sum_adaptation += Queue.top().adaptation;
				max_adaptation = max_adaptation > Queue.top().adaptation ? max_adaptation : Queue.top().adaptation;
				anrry.push_back(Queue.top());
				temp_anrry.push_back(Queue.top());//夫代染色体
				Queue.pop();
			}
			chromosome::max_adaptation = max_adaptation;
			chromosome::avg_adaptation = sum_adaptation / chromosome_number_max;
			//选择chromosome_number_max个染色体进入待选池子 END

			//杂交
			std::vector<chromosome> wait_to_cross;//杂交池
			//选择要进行杂交的进入杂交池//为后变异做准备，所以不能只留下交换后的染色体
			for (int i = 1; i <= chromosome_number_max; i++){
				if (anrry[i - 1].adaptation>chromosome::avg_adaptation){
					anrry[i - 1].PC = k1*sin(PI*(chromosome::max_adaptation - anrry[i - 1].adaptation) \
						/ (2 * (chromosome::max_adaptation - chromosome::avg_adaptation)));
				}
				else{
					anrry[i - 1].PC = k2;
				}
				if (rand() / double(RAND_MAX) < anrry[i - 1].PC){
					wait_to_cross.push_back(anrry[i - 1]);
				}
			}
			//进入杂交池 END
			//正式杂交
			while (wait_to_cross.size() >= 2){//如果只剩下一个那就不杂交了
				int x1 = rand() % wait_to_cross.size();
				chromosome temp_chromosome1 = wait_to_cross[x1];
				swap(wait_to_cross[x1], wait_to_cross[wait_to_cross.size() - 1]);
				wait_to_cross.pop_back();
				/*wait_to_cross.erase(wait_to_cross[x1 - 1]);*/
				int x2 = rand() % wait_to_cross.size();
				chromosome temp_chromosome2 = wait_to_cross[x2];
				swap(wait_to_cross[x2], wait_to_cross[wait_to_cross.size() - 1]);
				wait_to_cross.pop_back();

				temp_chromosome1.cross(temp_chromosome2);

			}
			//杂交 END

			//变异
			for (int i = 1; i <= chromosome_number_max; i++){
				if (anrry[i - 1].adaptation>chromosome::avg_adaptation){
					anrry[i - 1].PM = k3*sin(PI*(chromosome::max_adaptation - anrry[i - 1].adaptation) \
						/ (2 * (chromosome::max_adaptation - chromosome::avg_adaptation)));
				}
				else {
					anrry[i - 1].PM = k4;
				}
				if (rand() / double(RAND_MAX) < anrry[i - 1].PM){
					anrry[i - 1].variation();
				}
			}
			//变异 END

			//整合父子代
			for (int i = 1; i <= chromosome_number_max; i++){
				anrry.push_back(temp_anrry[i - 1]);
			}
			//整合父子代 END

		}
	A:
		f2 << "迭代次数： " << times << " 迭代过程中最优解: " << BEST_CHROME.using_time << endl;
		cout << "迭代次数： " << times << " 迭代过程中最优解: " << BEST_CHROME.using_time << endl;
		the_global_sum_time += BEST_CHROME.using_time;
		if (BEST_CHROME.using_time<the_best)the_best = BEST_CHROME.using_time;
		if (BEST_CHROME.using_time>the_worst)the_worst = BEST_CHROME.using_time;
		f1.close();
	}
	SYSTEMTIME sys2;
	GetLocalTime(&sys2);
	//time_t t2 = time(0);
	cout << "文件开启时间: " << sys1.wMinute << sys1.wMilliseconds << endl;
	cout << "文件关闭时间: " << sys2.wMinute << sys2.wMilliseconds << endl;
	f2 << "最好解: " << the_best << endl;
	f2 << "最坏解: " << the_worst << endl;
	f2 << "平均解: " << the_global_sum_time / test_times_max << endl;
	cout << "最好解: " << the_best << endl;
	cout << "最坏解: " << the_worst << endl;
	cout << "平均解: " << the_global_sum_time / test_times_max << endl;
	//f2 << "平均时间: " << ( 60*double(sys2.wHour-sys1.wHour)+(double(sys2.wMinute-sys1.wMinute)+(double(sys2.wMilliseconds - sys1.wMilliseconds)/1000000)))/ test_times_max << endl;
	f2 << "平均时间: " << (60 * double(sys2.wMinute - sys1.wMinute) + (double(sys2.wSecond - sys1.wSecond) + (double(sys2.wMilliseconds - sys1.wMilliseconds) / 1000000))) / test_times_max << endl;
	cout << "平均时间: " << ((60 * double(sys2.wMinute - sys1.wMinute) + double(sys2.wSecond - sys1.wSecond) + (double(sys2.wMilliseconds - sys1.wMilliseconds) / 1000000))) / test_times_max << endl;
	f2.close();
	return 0;
}