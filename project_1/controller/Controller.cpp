/** TODO: Transformar movimento em randômico
 *
 */

#include <Controller.hpp>

PlayerClient playerRobot("localhost");
Position2dProxy p2dProxy(&playerRobot, 0);
LaserProxy laserProxy(&playerRobot, 0);

Robot r;

int main()
{
    laserProxy.RequestGeom();
	
    while(true)
    {
		playerRobot.Read();
		
		// get current pose
		r.getTrueState();
		
		r.printInfoComparation();
		r.setVel(1);
		r.setRotVel(2);
		
		// set desired v and w
		move(r.getVel(), r.getRotVel());
		
		// update laser measurement array
		sense();
		
	}
	
	return 0;
}

void move(double v, double w)
{
	r.setVel(v);
	r.setRotVel(w);
	p2dProxy.SetSpeed(r.getVel(), r.getRotVel());
}

void sense()
{
	randomGaussianNoise(0.5);
	
	r.updateLaserArray();
	
	r.printLaserValue(5);
}

double randomGaussianNoise(double sigma)
{
	double gaussianNumber, randomNumber;
	
	randomNumber = (rand() % 5000)/5000;
	
	gaussianNumber = pow((-2*log(randomNumber*sigma*pow((2*M_PI),0.5))),0.5)*sigma;
	
	return gaussianNumber;
}
