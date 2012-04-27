#include <Controller.hpp>
// To do: Ajustar a média dos ângulos.

PlayerClient playerRobot("localhost");
Position2dProxy p2dProxy(&playerRobot, 0);
LaserProxy laserProxy(&playerRobot, 0);

Map worldMap, robotMap;

Robot r;

int main()
{
    laserProxy.RequestGeom();
	
	worldMap.createKnownMap();
	
    while(true)
    {
		r.updateState();
		
		r.printInfoComparison();
        
        /*
         * TODO: remove this setVel and setRotVel!
         * experimental circle, just to see if everything is OK.
         */
		strategy();
		
        
		move(r.getVel(), r.getRotVel());
		sense();
		kalmanFilter();
	}
	
	return 0;
}

void move(double v, double w)
{	
	r.setVel(v);
	r.setRotVel(w);
	
	if(v != 0)
	{
		v = randomGaussianNoise(r.getMoveVelSigma(), v);
	}
	if(w != 0)
	{
		w = randomGaussianNoise(r.getMoveRotVelSigma(), w);
	}
	
	p2dProxy.SetSpeed(v, w);
}

void sense()
{	
	r.updateLaserReadings();
	//r.printLaserReadings();
	//r.printValidLaserReadings();
    
    /*
     * TODO: Write interpretation function, which will do some analysis
     * with laser readings and return distances to walls and corners, as
     * well as to the landmark, when found.
     * 
     * interpretMeasurements()
     */
     
     interpretMeasurements();
}

bool interpretMeasurements()
{
	bool foundSomething;
	
	if (findLine() || findLandmark())
	{
		foundSomething = 1;
		if (findLine() == 2)
		{
			findCorner();
		}
		return 1;
	}
	
	
	return 0;
}

int findLine()
{
	unsigned int i, j, k;
	int number, counter;
	int oneTime;
	int found = 0;
	double deltaX, deltaY;
	double lineTheta, lineDistance;
	unsigned int sensorUsed;
	bool positiveAngle;
	vector<double> laserMeasurements = r.getLaserReadings();
	vector<int> validLaserMeasurements = r.getValidLaserReadings();
	vector<double> cosOfLine, cosMeans;
	vector<double> getPositions;
	mat houghMatrix = zeros<mat>(validLaserMeasurements.size(), 1000);
	
	cosMeans.clear();
	getPositions.clear();
	
	LOG(LEVEL_WARN) << "Finding Line Info";
	
	for (i=0;i<1000;i++)
	{
		for(j=0; j<validLaserMeasurements.size(); j++)
		{
			deltaX = laserMeasurements.at(validLaserMeasurements.at(j))*cos(r.getTh()+dtor((validLaserMeasurements.at(j)+180)%360));
			deltaY = laserMeasurements.at(validLaserMeasurements.at(j))*sin(r.getTh()+dtor((validLaserMeasurements.at(j)+180)%360));
			houghMatrix(j, i) = deltaX*cos(i*M_PI/1000) + deltaY*sin(i*M_PI/1000);
		}
	}
	
	for(j=0; j < validLaserMeasurements.size(); j++)
	{
		number = 0;
		cosOfLine.clear();
		for (i=0;i<1000;i++)
		{
			counter = 0;
			oneTime = 1;
			for (k=0; k<=100 && k <= validLaserMeasurements.size(); k++)
			{
				if (0.05 > abs(houghMatrix(j, i) - houghMatrix((j+k-50)%validLaserMeasurements.size(), i)) && k != 50)
				{
					counter = counter + 1;
					//LOG(LEVEL_INFO) << "Counter = " << counter << "\tk = " << k;
				}
			}
			if(counter > 60)
			{
				//LOG(LEVEL_INFO) << "Counter = " << counter << "\tj = " << j;
				//LOG(LEVEL_INFO) << "Counter = " << houghMatrix(1, 1) << "\tj = " << houghMatrix(1, 501);
				number = number + 1;
				cosOfLine.push_back(i);
			}
		}
		if (number>0)
		{
//			LOG(LEVEL_INFO) << "Number[" << validLaserMeasurements.at(j) << "] = " << number;
//			LOG(LEVEL_INFO) << "Theta[" << validLaserMeasurements.at(j) << "] = " << r.getTh();
//			LOG(LEVEL_INFO) << "Measurement Mean Theta[" << validLaserMeasurements.at(j) << "] = " << getMeanRoundWorld(cosOfLine, 1000);
			cosMeans.push_back(getMeanRoundWorld(cosOfLine, 1000));
			getPositions.push_back(validLaserMeasurements.at(j));
//			for (k=0; k<cosOfLine.size(); k++)
//				LOG(LEVEL_INFO) << "Measurement Theta[" << validLaserMeasurements.at(j) << "] = " << cosOfLine.at(k);
			found = 1;
		}
	}
	
	if (cosMeans.size() == 0)
		return 0;
	
	lineTheta = getMeanRoundWorld(cosMeans, 1000);
	sensorUsed = int(getMeanRoundWorld(getPositions, laserMeasurements.size()));
	
	deltaX = laserMeasurements.at(sensorUsed)*cos(r.getTh()+dtor((sensorUsed+180)%360));
	deltaY = laserMeasurements.at(sensorUsed)*sin(r.getTh()+dtor((sensorUsed+180)%360));
	lineDistance = deltaX*cos(lineTheta*M_PI/1000) + deltaY*sin(lineTheta*M_PI/1000);
	
	lineTheta = lineTheta*M_PI/1000;
	
	if (sensorUsed < laserMeasurements.size()/2)
		positiveAngle = 1;	//0-Pi => 1, 0-Pi => -1
	else
		positiveAngle = -1;
	
	LOG(LEVEL_INFO) << "Distance = " << lineDistance;
	LOG(LEVEL_INFO) << "Theta = " << positiveAngle*lineTheta;
	LOG(LEVEL_INFO) << "Sensor Used = " << sensorUsed;
	
	return 1;
}

bool findCorner()
{
	return 0;
}

bool findLandmark()
{
	return 0;
}

mat predictMean(mat A, mat B)
{
	mat ut = createUt();
	mat mu = createMu();
	mat muBar = A*mu + B*ut;
	
	
	
    /*
     * TODO: Write functions to build matrices and vectors to be used
     * by the Kalman Filter, such as mean, covariance, control and mea-
     * surements vectors, always using armadillo linear algebra library.
     * 
     *  mean = createMean();
     *  cov = createCov();
     *  ut = createUt();
     *  zt = createZt();
     *  Bt = updateBt();
     */
    
    return muBar;
}

mat predictiCov(mat A)
{
	mat R = createRt();
	mat sigma = r.getSigma();
	mat sigmaBar = A * sigma * A.t() + R;
	
	return sigmaBar;
}


mat createAt()
{
	mat A = eye<mat>(7,7);
	
	A(3, 3) = 0;
	A(4, 4) = 0;
	
	return A;
}

mat createBt()
{
	mat B = zeros<mat>(7,2);
	
	//Velocity parameters
	B(0, 0) = cos(r.getTh());
	B(1, 0) = sin(r.getTh());
	B(2, 0) = 0;
	B(3, 0) = 1;
	B(4, 0) = 0;
	B(5, 0) = 0;
	B(6, 0) = 0;
	
	//Rotation parameters
	B(0, 1) = 0;
	B(1, 1) = 0;
	B(2, 1) = 1;
	B(3, 1) = 0;
	B(4, 1) = 1;
	B(5, 1) = 0;
	B(6, 1) = 0;
	
	return B;

}

mat createUt()
{
	mat ut = zeros<mat>(2,1);
	
	ut(0,0) = r.getVel();
	ut(1,0) = r.getRotVel();
	
	return ut;
}

mat createMu()
{
	mat mu = zeros<mat>(7,1);
	
	mu(0,0) = r.getX();
	mu(1,0) = r.getY();
	mu(2,0) = r.getTh();
	mu(3,0) = r.getVel();
	mu(4,0) = r.getRotVel();
	mu(5,0) = 0;
	mu(6,0) = 0;
	
	return mu;
} 

mat createRt()
{
	mat R = eye<mat>(7,7);
	double theta = r.getTh();
	R(0,0) = r.getMoveVelSigma() * abs(cos(theta));
	R(1,1) = r.getMoveVelSigma() * abs(sin(theta));
	R(2,2) = r.getMoveRotVelSigma();
	R(3,3) = r.getMoveVelSigma();
	R(4,4) = r.getMoveRotVelSigma();
	R(5,5) = 0;
	R(6,6) = 0;
	
	return R;
}


void kalmanFilter()
{
	mat A = createAt();
	mat B = createBt();
	mat muBar = predictMean(A, B);
	mat sigmaBar = predictiCov(A);
	r.updateSigma(sigmaBar);	
	//
	//void postionUpdate(muBar, covBar)
    /*
     * TODO: Implement Kalman Filter algorithm.
     * TODO: Write functions to create each Kalman Filter matrix, such
     * as At, Bt and Ct, using armadillo.
     */
}

void strategy()
{
	int strategy_state = r.getStrategy();
	
	switch(strategy_state)
	{
		case 1://Estratégia de busca do canto
			
			bool detectObject = interpretMeasurements();
			
			if(detectObject == 1)
			{
				r.setVel(0);
				r.setRotVel(0);
			}
			else
			{
				r.setVel(0.5);
				r.setRotVel(0);
			}
		break;
	}
}
				
    /*
     * TODO: Write this function, which will choose the best action to
     * take after analysing all avaiable data.
     */


double randomGaussianNoise(double sigma, double mean)
{
	double gaussianNumber, randomNumber1, randomNumber2;
	
	randomNumber1 = double((rand() % 5000))/5000;
	randomNumber2 = double((rand() % 5000))/5000;
	
	while(randomNumber1 == 1 || randomNumber1 == 0)
	{
		randomNumber1 = double((rand() % 5000))/5000;
	}
	while(randomNumber2 == 1 || randomNumber2 == 0)
	{
		randomNumber2 = double((rand() % 5000))/5000;
	}
	
    gaussianNumber = pow(-2*log(randomNumber1),0.5)*cos(2*M_PI*randomNumber2)*sigma+mean;
	
	//LOG(LEVEL_WARN) << "Gaussian info";
	//LOG(LEVEL_INFO) << "Gaussian Number = " << randomNumber1;
	//LOG(LEVEL_INFO) << "Gaussian Number = " << randomNumber2;
	//LOG(LEVEL_INFO) << "Gaussian Number = " << gaussianNumber;
	
	return gaussianNumber;
}

double getMeanRoundWorld(vector<double> array, int worldSize)
{
	unsigned int counter;
	double mean1 = 0, mean2 = 0;
	double diff1 = 0, diff2 = 0;
		
	for (counter = 0; counter < array.size(); counter++)
	{
		if (array.at(counter) > worldSize/2)
			mean2 = mean2 + array.at(counter) - worldSize;
		else
			mean2 = mean2 + array.at(counter);
			
		mean1 = mean1 + array.at(counter);
	}
	
	mean1 = mean1 / array.size();
	mean2 = mean2 / array.size();
	
	for (counter = 0; counter < array.size(); counter++)
	{
		if (array.at(counter) > worldSize/2)
			diff2 = diff2 + abs (mean2 - array.at(counter) + worldSize);
		else
			diff2 = diff2 + abs (mean2 - array.at(counter));
		
		diff1 = diff1 + abs (mean1 - array.at(counter));
	}
	
	if(diff1 <= diff2)
		return mean1;
	else
	{
		if (mean2<0)
			return mean2+worldSize;
		else
			return mean2;
	}
}

double getMedian(vector<double> array)
{
	unsigned int counter;
	double mean = 0;
	
	for (counter = 0; counter < array.size(); counter++)
		mean = mean + array.at(counter);
	
	mean = mean / array.size();
	return mean;
}
