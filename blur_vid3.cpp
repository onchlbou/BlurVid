#include "opencv2/highgui/highgui.hpp" //interface video
#include <opencv2/imgproc/imgproc.hpp> //process d'images
#include "opencv2/objdetect/objdetect.hpp" //detection objets
#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>#include <unistd.h>
#include <termios.h>

/*
============================================================================
Nom : blur_vid3.c
Auteurs : Hue Nathan & Cagnon Charles
Version : 3
Copyright : non
Description : Applique un blur sur des visages d'une video/cam (24 Juin 2017)
============================================================================
*/

// g++ -w blur_vid3.cpp -o blu_vid3 `pkg-config --cflags --libs opencv`


#define DEBUG 1

using namespace cv;
using namespace std;
string titre = "blur";
int val=7;

// modification smoothing Temps Reel
int getVal(char msg){
   int val_int;
   switch(msg){
	case 'a':  val_int = val;                                  break; //blur homogene
	case 'b':  val_int = val; if (val_int%2==0){val_int++;}    break; //blur gaussien
	case 'c':  val_int = val; if (val_int%2==0){val_int++;}    break; //blur median
   }

   if (val>0 && val<50){
      return val_int;
   } else {
      val=9;
      return val;
   }
}


int main(void) {


    char msg[]="";
    cout << "Choissisez un type de blur:\n";
    cout << "[a]: blur homogene\n";
    cout << "[b]: blur Gaussien\n";
    cout << "[c]: blur Median\n";
    cout << "[d]: blur Filtre Bilateral\n";	
    cout << "--------------------------------- \n";
    cout << "En utilisation Temps reel\n";	
    cout << "tapotez p: Augmenter le smoothing\n";	
    cout << "tapotez o: Diminuer le smoothing\n";
    cout << "Changement de mode a,b,c possible\n";	
    scanf("%s",msg);


    CascadeClassifier haar_cascade;    
    //VideoCapture cap(0);       //WEBCAM
    VideoCapture cap("out.mp4"); //VIDEO

    //haar_cascade.load("haarcascade_frontalface_default.xml"); //Cascade peu performante
    haar_cascade.load("haarcascade_frontalface_alt.xml");       
 	
    // Test d'ouverture
    if ( !cap.isOpened() ) 
    {
        cout << "Impossible d'ouvrir le fichier \n";
        return -1;
    }    


    //lecture
    double fps = 0, temps_par_frame;
    Mat frame;
    while (1) {

       
        if (DEBUG) cout << "choix: " << msg[0] << " tpf: " << temps_par_frame <<" fps: " << fps << "val: " << val << endl;

	//testBench
	auto start = getCPUTickCount();
        cap >> frame;
        Mat original = frame.clone();
	auto end = getCPUTickCount();

        temps_par_frame = (end - start) / getTickFrequency();
        fps = (15 * fps + (1 / temps_par_frame)) / 16;
        
	//Copie grise de la video (traitement moins gourmand)
        Mat gray;
        cvtColor(original, gray, CV_BGR2GRAY);
        
	//Trouver des visages dans la video a partir d'un fichier cascade 
	//contenant les positions probables des traits du visages
        vector< Rect_<int> > faces;
	float facteur_echelle = 1.1;
	int voisins_minimum = 3;

        //haar_cascade.detectMultiScale(gray, faces); //qualité
	haar_cascade.detectMultiScale(gray, faces,    //performance
	facteur_echelle, 
	voisins_minimum, 
	0|CV_HAAR_SCALE_IMAGE,
	Size(frame.rows / 5, frame.rows / 5),
	Size(frame.rows * 2 / 3, frame.rows * 2 / 3));
	
	//Pour chaque visages detectés 
        for(int i = 0; i < faces.size(); i++) {

            Rect face_i = faces[i];
            Mat face = gray(face_i); //rogne le visgae
  
		//Application du type de smoothing    	    
		switch(msg[0]){
		    case 'a':  
			blur(original(face_i),original(face_i),Size(getVal(msg[0]),getVal(msg[0])));
			break;	
		    case 'b':  
			GaussianBlur(original(face_i),original(face_i), Size(getVal(msg[0]),getVal(msg[0])), 0, 0 ); 
			break;
		    case 'c':  
			medianBlur(original(face_i),original(face_i), getVal(msg[0]) );
			break;
		    case 'd':  
			cout << "Cette methode n'est toujours pas au point\n";
			//bilateralFilter(original(face_i),original(face_i), 0.5 ,0.5*2, 0.5/2 ); 
			break;
		    default:  break;	
		}
	    
	    //Rectangle vert d'epaisseur 1
            rectangle(original, face_i, CV_RGB(0, 255,0), 1); 

	    //Label au dessus des tetes
            string box_text = format("Temps par frame: %3.3f    fps: %3.3f", temps_par_frame,fps);
            int pos_x = std::max(face_i.tl().x - 10, 0);
            int pos_y = std::max(face_i.tl().y - 10, 0);
            putText(original, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);
        }
        // Afficher le resultat redimensionné:
	resize(original, original, Size(640, 360), 0, 0, INTER_CUBIC);
        imshow("Video "+titre, original);
        char key = (char) waitKey(20); //delais d'attente appui touche

        // 'ECHAP' Pour quitter la boucle
        if(key == 27)
            break;
	if(key == 97)
            msg[0]='a';
	    //titre = "blur";
	if(key == 98)
            //titre = "GaussianBlur";
            msg[0]='b';
	if(key == 99)
	    //titre = "medianBlur";
            msg[0]='c';
	//if(key == 100)
            //msg[0]='d';
	if(key == 112)
            val++;
	if(key == 111)
            val--;
    }
    return 0;
}

