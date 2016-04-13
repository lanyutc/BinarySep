#include "data_hide.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

int main()
{
	BinarySep<int32_t> int_a(256);
	printf("construct(int32_t) %d %d\n", int_a.get(), sizeof(int_a));

	int_a.hide(1);
	printf("int32_t hide %d\n", int_a.get());

	BinarySep<int32_t> int_b(int_a);
	printf("int32_t construct(T &t) %d\n", int_b.get());

	BinarySep<int32_t> int_c = int_a;
	printf("int32_t operator= %d\n\n", int_c.get());


	
	string stest = "123458";
	BinarySep<const char *> s(stest.c_str(), stest.size());
	printf("construct(string) %s\n", s.get());

	s.hide("5432111111", strlen("5432111111"));
	printf("string hide %s\n", s.get());

	s.hide("i am a test", 6);
	printf("string hide(less) %s\n", s.get());

	string ssst = s.get();
	cout << "char* to string " << ssst << endl;

	BinarySep<const char *> st(s);
	printf("string construct(T &t) %s\n", st.get());
	
	BinarySep<const char *> sst("content", 10);
	s = sst;
	printf("string operator= %s\n\n", s.get());



	
	BinarySep<double> d(3.141599222);
	printf("construct(double) %.10lf\n", d.get());

	d.hide(5.14156);
	printf("double result %.10lf\n", d.get());

	BinarySep<double> dd(d);
	printf("double construct(T &t) %.10lf\n", dd.get());

	BinarySep<double> ddd(7.14555);
	d = ddd;
	printf("double operator= %.10lf\n\n", d.get());



	BinarySep<bool> c(false);
	printf("construct(boll) %d\n", c.get() ? 1 : 0);

	c.hide(true);
	printf("bool hide %d\n\n", c.get() ? 1 : 0);

	
	vector<BinarySep<int32_t> > vb;
	BinarySep<int32_t> vb_a(100);
	BinarySep<int32_t> vb_b(200);
	BinarySep<int32_t> vb_c(300);
	vb.push_back(vb_a);
	vb.push_back(vb_b);
	vb.push_back(vb_c);
	printf("vector contain %d %d %d\n", vb[0].get(), vb[1].get(), vb[2].get());
	return 0;
}
