#include <iostream>
#include <ctime>
#include <vector>
#include <fstream>
#include <string>

#include <boost/json/src.hpp> 
#include <GL/freeglut.h>

#define DATA_VOLUME 20

//data structure
struct datakeeper
{
	int n;
	std::vector<float> vdata;
	void print(std::string comment)
	{
		std::cout << comment.c_str() << " - n =" << n << "; ";
		for (int i = 0; i < n; i++)
		{
			std::cout << "(" << i+1 << ") " << vdata[i] << "; ";
		}
		std::cout << std::endl;
	}
} values = {}, nvalues = {};

float nvd_min=0, nvd_max=0;

void datakeeper_extr(datakeeper vdt, float &min, float &max)
{
	min = vdt.vdata[0];
	max = vdt.vdata[0];

	for (int i = 0; i < vdt.n; i++)
	{
		if (min > vdt.vdata[i]) min = vdt.vdata[i];
		if (max < vdt.vdata[i]) max = vdt.vdata[i];
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float cw = 800.0f / (float)(nvalues.n-1);
	float ch = 600.0f / (nvd_max - nvd_min);

	glBegin(GL_LINE_STRIP);
	glColor3f(1.0f, 0.0f, 0.0f);
	for (int i = 0; i < nvalues.n; i++)
	{		
		glVertex2f((float)i*cw, (nvalues.vdata[i] - nvd_min)*ch);
	}
	glEnd();

	glutSwapBuffers();
}

float func(float a, float b, float c, float x)
{
	return a * pow(x, 2) + b * x + c;
}

void prep_data(int l, datakeeper &vdt)
{

	vdt.vdata.clear();

	float sx = 0.0f;
	float dx = 1.0f;
	float fa = 5;
	float fb = 2;
	float fc = -7;

	vdt.n = l;

	for (int i = 0; i < l; i++)
	{
		float x = sx + (float)i * dx;
		vdt.vdata.push_back(func(fa, fb, fc, x));
	}
}

void serialize_data(datakeeper vdt)
{
	//sample for json data serialization
	boost::json::value jv =
	{
		{"AMOUNT",vdt.n},
		{"VECTOR",vdt.vdata},
	};
	
	//create serializer object
	boost::json::serializer srlz;

	//assign value to serializer
	srlz.reset(&jv);

	//create filestream for saving data on HDD
	std::ofstream datawriter("datavault.json");

	//write data via a loop
	while (!srlz.done())
	{
		char buffer[BOOST_JSON_STACK_BUFFER_SIZE];
		datawriter << srlz.read(buffer);
	}

	datawriter.close();

}

void deserialize_data(datakeeper &vdt)
{
	vdt.vdata.clear();
	vdt.n = 0;

	std::ifstream datareader("datavault.json");
	std::string line = "", jstext = "";

	while (!datareader.eof())
	{
		std::getline(datareader,line);
		jstext += line;
	}

	datareader.close();

	//parse read json for data
	boost::json::stream_parser strp;
	strp.reset();
	strp.write(jstext);

	//write received json structre to value format
	boost::json::value fjv = strp.release(); 

	//drop data to proper structure
	vdt =
	{
		boost::json::value_to<int>(fjv.at("AMOUNT")),
		boost::json::value_to<std::vector<float>>(fjv.at("VECTOR"))
	};
}

int main(int argc, char **argv)
{
	//prepare opengl
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(200, 200);

	glutCreateWindow("Window");

	glClearColor(0, 0, 0, 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 800, 0, 600, -100, 100);
	glViewport(0, 0, 800, 600);

	srand(time(NULL));

	glutDisplayFunc(display);
	
	//prepare chart data
	prep_data(DATA_VOLUME, values);

	values.print("Generated data");

	//serialize data
	serialize_data(values);

	//deserialize data
	deserialize_data(nvalues);
	nvalues.print("Deserialized data");

	//get extremes
	datakeeper_extr(nvalues,nvd_min,nvd_max);

	//launch opengl main loop
	glutMainLoop();

	system("pause");
}