#include <stdio.h>
#include <cv.h>
#include <highgui.h>
//#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define PI 3.14159265

struct stack{
	int x;
	int y;
	int width;
	int height;
	int state;
	int index;
	int statej;
	int statei;
	int indexj;
	int indexi;
	int label;
};
struct cutinf{
	int bwcount;
	int blackmax;
	int whitemax;
	int height;
};
struct engdashnum{
	int numbernum;
	int englishnum;
	int label;//1左英右數;2右英左數 
};
typedef struct engdashnum engdashnum;
engdashnum platecharstate;
typedef struct stack stack;
stack *keyr,*keyfr,*keynew;

////////宣告結構////////////// 
typedef struct stack stack;
typedef struct cutinf cutinf;
cutinf *cuttop,*cutdown,*cutleft,*cutright;
stack *s1,*s2,*sT,u[2],*xmatrix=0,*ymatrix=0,*sv,*co_x=0,*co=0,*sht,*sh1=0,*sh2=0,*sst=0;
stack *area;
stack correctinf;
static counter=0;
/****************************/
/***************************************/
//轉彩色成灰階 
void colortograyimg(IplImage * sourceimg,IplImage * rusltImg) {
	int i,j;
	double Pixel = 0;
	for( i = 0;i < rusltImg->height;i ++)
       for( j = 0;j < rusltImg->width;j ++){
           Pixel = 0;
           Pixel += ((unsigned char*) (sourceimg->imageData))[i * sourceimg->widthStep + j * 3] * 0.114;
           Pixel += ((unsigned char*) (sourceimg->imageData))[i * sourceimg->widthStep + j * 3 + 1] * 0.587;
           Pixel += ((unsigned char*) (sourceimg->imageData))[i * sourceimg->widthStep + j * 3 + 2] * 0.299;
           ((unsigned char*) (rusltImg->imageData))[i * rusltImg->widthStep + j] = (int)Pixel;

       }/* */
    /*cvShowImage("HelloWorld-2",rusltImg);
    cvWaitKey(0);*/
	return;  
}
////備份影像////
void backupimg(IplImage * grayImg,IplImage * oldimg_v) {
	int i,j;
	for( i = 0;i < grayImg->height;i ++)
       for( j = 0;j < grayImg->width;j ++)
           oldimg_v->imageData[i *  oldimg_v->widthStep + j ]=grayImg->imageData[i *  grayImg->widthStep + j ];
   /* cvShowImage("HelloWorld-110",oldimg_v);
    cvWaitKey(0);  */     
}
////一階微分影像處理////
void gradiimg(IplImage * testimg,int* test){ 
   int i,j;
  for( i = 0;i < testimg->height;i ++)
     for( j = 0;j < testimg->width;j ++){
  	   // gradiimg->imageData[i * gradiimg->widthStep + j]=(testimg->imageData[i * testimg->widthStep + (j+1)]-testimg->imageData[i * testimg->widthStep + j]);
    //testimg->imageData[i *  testimg->widthStep + j ]=((unsigned char*)(testimg->imageData))[(i+0) * testimg->widthStep + (j+1)]-((unsigned char*)(testimg->imageData))[i * testimg->widthStep + j];
	test[i *  testimg->widthStep + j ]=((unsigned char*)(testimg->imageData))[(i+0) * testimg->widthStep + (j+1)]-((unsigned char*)(testimg->imageData))[i * testimg->widthStep + j];
   
    //test[i *  testimg->widthStep + j ]=testimg->imageData[(i+0) * testimg->widthStep + (j+1)]-testimg->imageData[i * testimg->widthStep + j];
    //testimg->imageData[i *  testimg->widthStep + j ]=((testimg->imageData))[(i+0) * testimg->widthStep + (j+1)]-((testimg->imageData))[i * testimg->widthStep + j];
	}
} 
//二值化演算法 //
void graytobinary(int Tmax,IplImage * testimg) {
	int i1,j1,i,j;
        for( i = 0;i < testimg->height;i ++)
       for( j = 0;j < testimg->width;j ++){
          if(( (unsigned char *)/**/testimg->imageData)[i *  testimg->widthStep + j ] < Tmax)
             testimg->imageData[i *  testimg->widthStep + j ]=0;
          else
             testimg->imageData[i *  testimg->widthStep + j ]=255;  
       }
       
}/**/
//得適當threshod值// 
int suitthreshod(IplImage * testimg,int* test,float r){
	int i,j,max=0;
	double avge=0,sum=0;
	int T=0;
	for( i = 0;i < testimg->height;i ++){
      for( j = 0;j < testimg->width;j ++){
  	     if(abs(test[i *  testimg->widthStep + j ] )>  max )
  	        max=abs(test[i *  testimg->widthStep + j ]);
      }
   sum=sum+max;
   max=0;
   }
   avge=sum/testimg->width;
   T=avge*r;
   return T;
}
//得到絕對值一階微分影像//
void absgradimg(IplImage * testimg,int* test){
	int i,j;
	for( i = 0;i < testimg->height;i ++)
      for( j = 0;j < testimg->width;j ++){
        //testimg->imageData[i *  testimg->widthStep + j ]=abs(testimg->imageData[i *  testimg->widthStep + j ]);
  	     testimg->imageData[i *  testimg->widthStep + j ]=abs(test[i *  testimg->widthStep + j ]);///------------G(x,y)=|g(x,y+1)-g(x,y)|
  	
      }
} 
//整體密度// 
 double wholedensity(IplImage * testimg){ 
    int i,j;
    unsigned int count=0;
    double w=0;
    for( i = 0;i < testimg->height;i ++)
       for( j = 0;j < testimg->width;j ++)
          if( ( (unsigned char *)testimg->imageData)[i * testimg->widthStep + j]==255)
               count=count+1;
    
    w=(double)count/(testimg->width*testimg->height);
   // printf("count:%d\n",count);
   // printf("w:%lf\n",w);
    return w;
} 

///連通法(計數次數)///
 int conponentnum(IplImage * testimg,int * test) {
 	int i,j,numa=0;
 	CvMemStorage * storage = cvCreateMemStorage(0);
 	CvSeq * contour = 0;
    int mode = CV_RETR_EXTERNAL;
 	 for( i = 0;i < testimg->height;i ++)
     for( j = 0;j < testimg->width;j ++){
  	    
        test[i *  testimg->widthStep + j ]=testimg->imageData[i *  testimg->widthStep + j ];
     }
     
  cvFindContours( testimg, storage, &contour, sizeof(CvContour), 
		  mode, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
	for( i = 0;i < testimg->height;i ++)
     for( j = 0;j < testimg->width;j ++){
  	    
        testimg->imageData[i *  testimg->widthStep + j ]=test[i *  testimg->widthStep + j ];
     }
      
while(contour != NULL){
     numa++;
     contour = contour -> h_next;
}	 	 	 
return numa;
}
///連通法標記///
int labelconponent(IplImage * cutImg,CvRect *ect ) {
	 int num1=0;
	 CvMemStorage * storage = cvCreateMemStorage(0);
 	CvSeq * contour = 0;
    int mode = CV_RETR_EXTERNAL;
	 cvFindContours( cutImg, storage, &contour, sizeof(CvContour), 
		  mode, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
	while(contour != 0){
		
	      ect[num1] = cvBoundingRect( contour, 0 );
	      num1++;
	      contour = contour -> h_next;
	}/**/
	return num1;
}

///區塊氣泡排序1///
void areasort(CvRect * ect,int numa){
	int i,j;
	int temp,temp2,temp3,temp4;
		for(j=0;j<numa-1;j++)
       for(i=0;i<numa-j-1;i++){
       	  if(ect[i].x > ect[i+1].x){
       	  	 temp   =ect[i].x;
       	  	  temp2 =ect[i].y;
       	  	  temp3 =ect[i].width;
       	  	   temp4 =ect[i].height;
       	  	 ect[i].x=ect[i+1].x;
       	  	 ect[i].y=ect[i+1].y;
       	  	 ect[i].width=ect[i+1].width;
       	  	 ect[i].height=ect[i+1].height;
       	  	 ect[i+1].x=temp;
       	  	 ect[i+1].y=temp2;
       	  	 ect[i+1].width=temp3;
       	  	 ect[i+1].height=temp4;
       	  	 
			}
       	    
	   }
}
///區塊氣泡排序2///
void areasort1(stack * ect,int numa){
	int i,j;
	int temp,temp2,temp3,temp4;
		for(j=0;j<numa-1;j++)
       for(i=0;i<numa-j-1;i++){
       	  if(ect[i].x > ect[i+1].x){
       	  	 temp   =ect[i].x;
       	  	  temp2 =ect[i].y;
       	  	  temp3 =ect[i].width;
       	  	   temp4 =ect[i].height;
       	  	 ect[i].x=ect[i+1].x;
       	  	 ect[i].y=ect[i+1].y;
       	  	 ect[i].width=ect[i+1].width;
       	  	 ect[i].height=ect[i+1].height;
       	  	 ect[i+1].x=temp;
       	  	 ect[i+1].y=temp2;
       	  	 ect[i+1].width=temp3;
       	  	 ect[i+1].height=temp4;
       	  	 
			}
       	    
	   }
}
//合併後座標// 
stack mergerarea(int i,int j,CvRect * ect){
	stack merger;
	int xmin,ymin,xmaxlen,ymaxlen;
//-----------合併左上x,y點-------------
	if(ect[j].x < ect[i].x) 
		xmin=ect[j].x;
	else
		xmin=ect[i].x;
				   
	if(ect[j].y < ect[i].y) 
		ymin=ect[j].y;
	else
	    ymin=ect[i].y; 
//-----------合併x,y點之長高----------------     
	if(ect[j].x + ect[j].width < ect[i].x + ect[i].width ) 
		xmaxlen=(ect[i].x + ect[i].width) - xmin;
	else
		xmaxlen=(ect[j].x + ect[j].width) - xmin; 
				       
	if(ect[j].y + ect[j].height < ect[i].y + ect[i].height ) 
		ymaxlen=(ect[i].y + ect[i].height) - ymin;
	else
	    ymaxlen=(ect[j].y + ect[j].height) - ymin; 
	merger.x=xmin;
	merger.y=ymin;
	merger.width=xmaxlen;
	merger.height=ymaxlen;
	return merger;
}
//相鄰合併// 
int neighbormerger( int k,int num,int count3,CvRect *ect,stack* keyr){
	int event,Condition3,Condition2,Condition,p1_hy,p2_hy;
	int refheight,h,diff_h,diff_p1y_h,diff_p2y_h,p1_hx,p2_hx,diff_p1x_h,diff_p2x_h;
	int xmin,ymin,xmaxlen,ymaxlen,dx=0;
	int i,j,xyn=0,index=0;
	      for(j=0;j<num;j++){
          for(i=0;i<num;i++){
	         
	         event=0;
	         Condition3=0;
	         Condition2=0;
	         Condition=0;
	         
             p1_hy=ect[j].y+ect[j].height;
             p2_hy=ect[i].y+ect[i].height;
             diff_p1y_h=p1_hy-ect[i].y;
             diff_p2y_h=p2_hy-ect[j].y;
           
             p1_hx=ect[j].x+ect[j].width;
             p2_hx=ect[i].x+ect[i].width;
             diff_p1x_h=ect[j].x-p2_hx;
             diff_p2x_h=ect[i].x-p1_hx;
          	 if(k%2!=0){
			   
            //(1)由右到左掃描
               //(2)由參考之左邊小於選擇右邊(重疊) 
            if(ect[j].x >= p2_hx && diff_p1x_h>=0 ) {
		         dx= ect[j].x-p2_hx;
		         event=1;
			 } 
			 
			 else if( ect[j].x < p2_hx && diff_p1x_h<=0 && p1_hx > p2_hx) {
		        dx= ect[j].x-p2_hx;
		        event=2;
			 } 
			}
			else{	  
		
			 //if()
		     //(1)由左到右掃描 
			   //(2)由參考之右邊大於選擇左邊 (重疊) 
           	 if(p1_hx <= ect[i].x && diff_p2x_h>=0){
           	     dx=ect[i].x-p1_hx;
           	     event=3;
           	    
			 } 	
		     else if(p1_hx >= ect[i].x && diff_p2x_h<=0 && ect[j].x < ect[i].x) {
		          dx=ect[i].x-p1_hx;
		          event=4;
			 }
			} 
				
             if(event==1 || event==2 || event==3 || event==4 )
                if( ( abs(dx)/**/ <= (float)ect[j].width*0.3+1)  && j!=i ){
                	///處理各種情況高度數學模型 
                	//處理介於高度的區塊
                   if(ect[j].height > ect[i].height ){
				   
                      if(ect[j].y <= ect[i].y && p1_hy >= p2_hy){
                      	 Condition2=1;
                      	 refheight=ect[i].height;
					  }
                   }     
                   else{
                   	  if(ect[i].y <=ect[j].y && p2_hy >= p1_hy){
                      	 Condition2=2;
                      	 refheight=ect[j].height;
					  }
                   	
				   }
                      				   
		        //處理介於參考高度的區塊 
			       if(Condition2){
		       	      h=refheight;
		       	      event=5;
			       } 
			       else{
			       	
			       	        //(一)由上到下移動(選擇I) 
							  //(1)參考J高度>=選擇I高度
							  //(2)參考J頂座標>選擇I頂座標
							  //(3)參考J頂座標<=選擇I底座標
							  //----(4)參考J頂座標>選擇I頂座標---- 
							  //(二)由上到下移動(選擇I) 
							  //(1)參考J高度<選擇I高度
							  //(2)參考J頂座標>選擇I頂座標
							  //(3)參考J頂座標<=選擇I底座標 
							  //(4)參考J底座標>選擇I底座標
					        
										   			    
			       	    if(ect[j].height >= ect[i].height ){
			       	       if(ect[j].y > ect[i].y) 
			       	       	  if(ect[j].y <= p2_hy){
			       	       	  	diff_h=p2_hy-ect[j].y;
			       	       	  	Condition3=1;
			       	       	  	event=6;
							  }
			       	          	 
			       	          	
						          
		                }
		                
		                else if(ect[j].height < ect[i].height){
		                        if(ect[j].y > ect[i].y) 
			       	               if( ect[j].y <= p2_hy)
			       	                  if(p1_hy > p2_hy){
			       	                  	diff_h=p2_hy-ect[j].y;
			       	                  	Condition3=2;
			       	                  	event=7;
									  }
			       	                                   
		                }
		                
		                      //(一)由下到上移動(選擇I) 
							  //(1)參考J高度>=選擇I高度
							  //(2)參考J頂座標<選擇I頂座標
							  //(3)參考J底座標>=選擇I頂座標
							  //(4)參考J底座標<選擇I底座標
							  //(二)由下到上移動(選擇I) 
							  //(1)參考J高度<選擇I高度
							  //(2)參考J頂座標<選擇I頂座標
							  //(3)參考J底座標>=選擇I頂座標 
							  //--(4)參考J頂座標<選擇I頂座標--- 
							  
					    
					                 
		                if(ect[j].height >= ect[i].height ){
			       	       if(ect[j].y < ect[i].y) 
			       	          if( p1_hy>=ect[i].y )
			       	             if(p1_hy < p2_hy){
			       	             	diff_h=p1_hy-ect[i].y;
			       	             	Condition3=1;
			       	             	event=8;
								 }
									 
						           
		                }
		                 
		                else if(ect[j].height < ect[i].height){
		                        if(ect[j].y < ect[i].y) 
			       	               if( p1_hy>= ect[i].y){
			       	               	diff_h=p1_hy-ect[i].y;
			       	               	Condition3=2;
			       	               	event=9;
								   }		
		                }
		           }
			       ///結束  
			      
           	       ///辨斷高度 
           	       if(event==5){
					  
           	          if(Condition2==1) 
           	             Condition= (float)h/ect[j].height >= 0.4   &&  1>= (float)h/ect[j].height;
           	          else if(Condition2==2) 
           	            Condition= (float)h/ect[i].height >= 0.4   &&  1>= (float)h/ect[i].height;
           	       } 
           	       else
					  if(event==6 || event==7 || event==8|| event==9){
					  	if(Condition3==1) 
           	               Condition= (float)diff_h/ect[j].height >= 0.6   &&  1>= (float)diff_h/ect[j].height;
           	            else if(Condition3==2) 
           	               Condition= (float)diff_h/ect[i].height >= 0.6   &&  1>= (float)diff_h/ect[i].height;
					  }
           	             
           	        ///結束    
					         
			        if(Condition){
				       //-------------合併演算法------------------
				       //-----------合併左上x,y點-------------
				
				         if(ect[j].x < ect[i].x) 
				            xmin=ect[j].x;
				         else
				            xmin=ect[i].x;
				   
				         if(ect[j].y < ect[i].y) 
				            ymin=ect[j].y;
				         else
				            ymin=ect[i].y; 
				       //-----------合併x,y點之長高----------------     
				         if(ect[j].x + ect[j].width < ect[i].x + ect[i].width ) 
				            xmaxlen=(ect[i].x + ect[i].width) - xmin;
				         else
				            xmaxlen=(ect[j].x + ect[j].width) - xmin; 
				       
				         if(ect[j].y + ect[j].height < ect[i].y + ect[i].height ) 
				            ymaxlen=(ect[i].y + ect[i].height) - ymin;
				         else
				            ymaxlen=(ect[j].y + ect[j].height) - ymin;      
				     
				        
				//--------------------顯示----------------------- 
				        
				//------------紀錄處理---------------------- 
				         
			    //------------結束------------------------------------ 
			    //------------合併暫存----------------------
			    
			      xyn++;
		          count3++; 
		          if(xyn>1){//只每次獨立合併一次 
		          	index=1;	
				  }
                //-----------結束------------------------------------ 
			  
			    
				//-------------結束------------------
			         }  
                 }
                 //------------合併暫存----------------------
                 if(Condition && index!=1){//記錄合併頭(keyr[])，且合併所有可能區域。存放在keyr[]作記錄。 
		            keyr[count3-1].x=xmin;
		            keyr[count3-1].y=ymin;
		            keyr[count3-1].width=xmaxlen;
		            keyr[count3-1].height=ymaxlen;
		            keyr[count3-1].state=0;
		            keyr[count3-1].index=j;
		            keyr[count3-1].statej=1;
		            keyr[count3-1].statei=-1;
		            keyr[count3-1].indexj=j;
		            keyr[count3-1].indexi=i;
				 }
			  
          }
         
          xyn=0;
          index=0;
          
       }
        
       //printf("count3:%d\n",count3);
	
    return count3;  
}
//重疊合併處理//
void mergeoverlap(int count3,stack* keyr,stack* keyfr){
	int jj,ii,tempminx,tempminy,tempw,temph;
	int event,Condition3,Condition2,Condition,p1_hy,p2_hy;
	int refheight,h,diff_h;
	int xmin,ymin,xmaxlen,ymaxlen;
	for(jj=0;jj<count3;jj++){   
		           	tempminx=keyr[jj].x;
		            tempminy=keyr[jj].y;
		            tempw=keyr[jj].width;
		            temph=keyr[jj].height;
            for(ii=0;ii<count3;ii++){
            	event=0;
	            Condition3=0;
	            Condition2=0;
	            Condition=0;
	            p1_hy=keyr[jj].y+keyr[jj].height;
                p2_hy=keyr[ii].y+keyr[ii].height;
	         if(tempminx <= keyr[ii].x && (tempminx+tempw) >=keyr[ii].x && ii!=jj){
			     /*處理介於高度的區塊*/
                    if(keyr[jj].height > keyr[ii].height ){
				   
                      if(keyr[jj].y <= keyr[ii].y && p1_hy >= p2_hy){
                      	 Condition2=1;
                      	 refheight=keyr[ii].height;
					  }
                   }     
                   else{
                   	  if(keyr[ii].y <=keyr[jj].y && p2_hy >= p1_hy){
                      	 Condition2=2;
                      	 refheight=keyr[jj].height;
					  }
                   	
				   }
                      				   
		        /*處理介於參考高度的區塊*/ 
			       if(Condition2){
		       	      h=refheight;
		       	      event=5;
			       } 
			       else{
			       	
			       	        /*(一)由上到下移動(選擇I) 
							  (1)參考J高度>=選擇I高度
							  (2)參考J頂座標>選擇I頂座標
							  (3)參考J頂座標<=選擇I底座標
							  ----(4)參考J頂座標>選擇I頂座標---- 
							  (二)由上到下移動(選擇I) 
							  (1)參考J高度<選擇I高度
							  (2)參考J頂座標>選擇I頂座標
							  (3)參考J頂座標<=選擇I底座標 
							  (4)參考J底座標>選擇I底座標
					        */
										   			    
			       	    if(keyr[jj].height >= keyr[ii].height ){
			       	       if(keyr[jj].y > keyr[ii].y) 
			       	       	  if(keyr[jj].y <= p2_hy){
			       	       	  	diff_h=p2_hy-keyr[jj].y;
			       	       	  	Condition3=1;
			       	       	  	event=6;
							  }
			       	          	 
			       	          	
						          
		                }
		                
		                else if(keyr[jj].height < keyr[ii].height){
		                        if(keyr[jj].y > keyr[ii].y) 
			       	               if( keyr[jj].y <= p2_hy)
			       	                  if(p1_hy > p2_hy){
			       	                  	diff_h=p2_hy-keyr[jj].y;
			       	                  	Condition3=2;
			       	                  	event=7;
									  }
			       	                                   
		                }
		                
		                /*    (一)由下到上移動(選擇I) 
							  (1)參考J高度>=選擇I高度
							  (2)參考J頂座標<選擇I頂座標
							  (3)參考J底座標>=選擇I頂座標
							  (4)參考J底座標<選擇I底座標
							  (二)由下到上移動(選擇I) 
							  (1)參考J高度<選擇I高度
							  (2)參考J頂座標<選擇I頂座標
							  (3)參考J底座標>=選擇I頂座標 
							  --(4)參考J頂座標<選擇I頂座標--- 
							  
					    */
					                 
		                if(keyr[jj].height >= keyr[ii].height ){
			       	       if(keyr[jj].y < keyr[ii].y) 
			       	          if( p1_hy>=keyr[ii].y )
			       	             if(p1_hy < p2_hy){
			       	             	diff_h=p1_hy-keyr[ii].y;
			       	             	Condition3=1;
			       	             	event=8;
								 }
									 
						           
		                }
		                 
		                else if(keyr[jj].height < keyr[ii].height){
		                        if(keyr[jj].y < keyr[ii].y) 
			       	               if( p1_hy>= keyr[ii].y){
			       	               	diff_h=p1_hy-keyr[ii].y;
			       	               	Condition3=2;
			       	               	event=9;
								   }		
		                }
		           }
			       ///辨斷高度 
           	       if(event==5){
					  
           	          if(Condition2==1) 
           	             Condition= (float)h/keyr[jj].height >= 0   &&  1>= (float)h/keyr[jj].height;
           	          else if(Condition2==2) 
           	            Condition= (float)h/keyr[ii].height >= 0   &&  1>= (float)h/keyr[ii].height;
           	       } 
           	       else
					  if(event==6 || event==7 || event==8|| event==9){
					  	if(Condition3==1) 
           	               Condition= (float)diff_h/keyr[jj].height >= 0.6  &&  1>= (float)diff_h/keyr[jj].height;
           	            else if(Condition3==2) 
           	               Condition= (float)diff_h/keyr[ii].height >= 0.6   &&  1>= (float)diff_h/keyr[ii].height;
					  }
			       
			       
				    
	        	   
	         	  if(Condition){
	         	  	 
	         	  	 
	         	  	 if(keyfr[jj].state==-1 ){
	             	//flag=1;
	                break;	
				 }
	              else{
	              	keyfr[jj].state=1;
	              	keyfr[jj].statej=1;
	              	keyfr[jj].indexj=jj;
				  }
	                 
	                 
	         	   	 //keyr[ii].state=-1;
	         	     keyfr[ii].state=-1;
					 keyfr[ii].statei=-1;
					 keyfr[ii].indexi=ii;        
	         	     if(tempminx < keyr[ii].x) 
				            xmin=tempminx;
				         else
				            xmin=keyr[ii].x;
				   
				         if(tempminy < keyr[ii].y) 
				            ymin=tempminy;
				         else
				            ymin=keyr[ii].y; 
				       //-----------重疊合併x,y點之長高----------------     
				         if(tempminx + tempw < keyr[ii].x + keyr[ii].width ) 
				            xmaxlen=(keyr[ii].x + keyr[ii].width) - xmin;
				         else
				            xmaxlen=(tempminx + tempw) - xmin; 
				       
				         if(tempminy + temph < keyr[ii].y + keyr[ii].height ) 
				            ymaxlen=(keyr[ii].y + keyr[ii].height) - ymin;
				         else
				            ymaxlen=(tempminy + temph) - ymin; 
							 
						 
	         	        tempminx=xmin;
	         	        tempminy=ymin;
	         	        temph=ymaxlen;
	         	        tempw=xmaxlen;
	         	        
	         	        
	         	        
	         	   }
	         	   
	         	   
	         	 }
	        
	            
	        } 	        
			               if(keyfr[jj].state==1){
			               	keyfr[jj].x=tempminx;
          	               keyfr[jj].y=tempminy;
          	               keyfr[jj].width=tempw;
          	               keyfr[jj].height=temph; 
						   }
			               
					
	               	       
									          
		}	
	
	
}
void neighbordatarenew(CvRect * ect,stack* keyr,int num){
	int i;
	for(i=0;i<num;i++){
        ect[keyr[i].index].x=keyr[i].x;
	    ect[keyr[i].index].y=keyr[i].y;
	    ect[keyr[i].index].width=keyr[i].width;
	    ect[keyr[i].index].height=keyr[i].height;
	    	
	 }	
}
void overlapdatarenew(CvRect * ect,stack* keyr,stack* keyfr,int num){
	int i;
	for(i=0;i<num;i++)
	 if(keyfr[i].state==1 ) {//合併頭	
	    ect[keyr[i].index].x=keyfr[i].x;
	    ect[keyr[i].index].y=keyfr[i].y;
	    ect[keyr[i].index].width=keyfr[i].width;
	    ect[keyr[i].index].height=keyfr[i].height;
     }	
}
//計算更新刪減數處理//
int renewdeletenum(int num,CvRect * ect,stack* keyr,stack* keyfr,int* indextest){
	int jj,ii,i,event,Condition3,Condition2,Condition;
	int cont=0;
	for(jj=0;jj<num;jj++)
            for(ii=0;ii<num;ii++){
	         event=0;
	         Condition3=0;
	         Condition2=0;
	         Condition=0;
	        if(keyfr[jj].state==1 ) {//合併頭	
	        	ect[keyr[jj].index].x=keyfr[jj].x;
	        	ect[keyr[jj].index].y=keyfr[jj].y;
	        	ect[keyr[jj].index].width=keyfr[jj].width;
	        	ect[keyr[jj].index].height=keyfr[jj].height;/**/
	        	///計算有幾個要刪減區塊/// --->合併內小區塊刪除 
	        	if (keyfr[jj].x<=ect[ii].x && keyr[jj].index!=ii )
		          if (keyfr[jj].x+keyfr[jj].width>=ect[ii].x+ect[ii].width)        
		             if (keyfr[jj].y<=ect[ii].y)   
		                if (keyfr[jj].y+keyfr[jj].height>=ect[ii].y+ect[ii].height) /* */{  
		               	   cont++;
		               	   indextest[cont-1]=ii; 
					   } 
			}
	}
/*	for(i=0;i<num;i++){
		keyfr[i].state=0;
		keyfr[i].x=0;
        keyfr[i].y=0;
        keyfr[i].width=0;
        keyfr[i].height=0;
	}*/
	return cont;
}
//更新刪減處理//
int deletearea(int cont,int num,CvRect * ect,stack* keynew ,int* indextest){
	int j,i=0,cont2=0,gg=0;
		for(j=0;j<=cont;j++){
		   for(i=i;i<num;i++){
		       if(indextest[j]!=i){   
		       	  cont2++;
		       	  keynew[cont2-1].x=ect[i].x;
		          keynew[cont2-1].y=ect[i].y;
		          keynew[cont2-1].width=ect[i].width;
		          keynew[cont2-1].height=ect[i].height;/**/
		         /* ect[i].x=0;
		          ect[i].y=0;
		          ect[i].width=0;
		          ect[i].height=0;*/
		          
			 }
		   	 else{
		   	 	i=indextest[j]+1;
		   	 	break;
				}
		   	    
		   }
		 }  
/*	for(gg=0;gg<cont;gg++){
		indextest[i]=0;
	}*/	 
	
	return cont2;	 
}
//更新處理//
int renewarea(int num,int cont2,CvRect * ect,stack* keynew){
	int j,i;
	for(j=0;j<num;j++){   	  
		       	  ect[j].x=0;
		          ect[j].y=0;
		          ect[j].width=0;
		          ect[j].height=0;
		   }/**/
		  for(j=0;j<cont2;j++){   	  
		       	  ect[j].x=keynew[j].x;
		          ect[j].y=keynew[j].y;
		          ect[j].width=keynew[j].width;
		          ect[j].height=keynew[j].height;
		   }
		 /*  for(i=0;i<cont2;i++){
		   	      keynew[i].x=0;
		          keynew[i].y=0;
		          keynew[i].width=0;
		          keynew[i].height=0;
		   }*/
		          
		   num=cont2;
		   
	return num;	   
}
int otsu(IplImage * blockImg) {
	//////////OTSU演算法 //////////
	int counter=0;
	int counter_1=0,max=0;
	int i1=0,j1=0,k1=0,i=0,tempT=0,L=0,Tmax=0;
	double p[256],W1=0,W2=0,u0=0,u1=0,uT=0;
	double sigmoid0=0,sigmoid1=0,sigmoidB=0,sigmoidW=0,Coef;
	
	//////////圖片總數//////////
    for( i1 = 0;i1 < blockImg->height;i1 ++)
        for( j1 = 0;j1 < blockImg->width;j1 ++){
  	        counter++;	
    }/* */
   
        //////////各灰階佔整總數的密度//////////
    for( k1 = 0;k1 <= 256;k1 ++){
        for( i1 = 0;i1 < blockImg->height;i1 ++){
            for( j1 = 0;j1 < blockImg->width;j1 ++){
                if(((unsigned char*)(blockImg->imageData))[i1 * blockImg->widthStep + j1 ]==k1)
                   counter_1++;               
            }                
        }
        p[k1]=(double)counter_1/counter;        
        counter_1=0;
    } /*	*/            
        //////////類別密度計算//////////
    for(tempT=0;tempT<256;tempT++){ 
        for(L=0;L<256;L++){                                         
           if(L<tempT)
              W1=W1+p[L]; 
           else
              W2=W2+p[L];     
        } 
		if(W1!=0 && W2!=0){ /* */ 
                           
         //////////平均計算//////////
           for(L=0;L<256;L++){ 
               if(L<tempT)
                  u0 = u0+(L)* p[L]/W1; 
           else
                  u1 = u1+(L)* p[L]/W2;            
           }  
           for(i=0;i<256;i++){
               uT = uT+(i)* p[i];            
           } /*  */
     
        //////////變異數計算//////////
           for(L=0;L<256;L++){  
              if(L<tempT)
                 sigmoid0 = sigmoid0+pow(((L)-u0),2)*p[L]/W1; 
              else
                 sigmoid1 = sigmoid1+pow(((L)-u1),2)*p[L]/W2;    
           }  
           sigmoidB = W1*pow((u0-uT),2)+W2*pow((u1-uT),2);            
           sigmoidW = W1*pow(sigmoid0,0.5)+W2*pow(sigmoid1,0.5);                
           Coef=sigmoidB/sigmoidW;
           if(max<Coef){
              max=Coef;
              Tmax=tempT; 
           } 
        }
        W1=0;
		W2=0;
		u0=0;
		u1=0;
		uT=0;
		sigmoid0=0;
		sigmoid1=0;
    }  /**/
   return Tmax;	
}

void correct( stack * matrix,stack* sT,int counts1,double thear,int event,int deltx,int delty){
	int i,x1,y1;
	if(event==1){
		for(i=0;i<counts1;i++){
	    x1=(int)(sT[i].x*cos((thear)*PI/180)-sT[i].y*sin((thear)*PI/180));
	    y1=(int)(sT[i].x*sin((thear)*PI/180)+sT[i].y*cos((thear)*PI/180));
	    matrix[i].x=x1;
	    matrix[i].y=y1; 	  
	    }
		
	}
	else if(event==2){
		for(i=0;i<counts1;i++){
	        x1=(int)(sT[i].x);
	        y1=(int)((sT[i].y+delty)+(sT[i].x+deltx)*(thear)*PI/180);
	        matrix[i].x=x1;
	        matrix[i].y=y1;   
	    
	   }
	}
	
}

///連通法segment(計數次數)///
 int conponentnumseg(IplImage * cuttest,IplImage * cutImg) {
 	int i,j,numa=0;
 	CvMemStorage * storage = cvCreateMemStorage(0);
 	CvSeq * contour = 0;
    int mode = CV_RETR_EXTERNAL;
 		for( i = 0;i < cutImg->height;i ++)
     for( j = 0;j < cutImg->width;j ++){
  	    
        cuttest->imageData[i *  cuttest->widthStep + j ]=cutImg->imageData[i *  cutImg->widthStep + j ];
     }
     
 cvFindContours(  cutImg, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
 
 for( i = 0;i < cutImg->height;i ++)
     for( j = 0;j < cutImg->width;j ++){
  	    
        cutImg->imageData[i *  cutImg->widthStep + j ]=cuttest->imageData[i *  cuttest->widthStep + j ];
     }   
while(contour != NULL){
     numa++;
     contour = contour -> h_next;
}
return numa;
}

//變成背黑前白處理參數顯示//
void parameterplateimg(IplImage * grayImg) {
	int count=0;
	int count2=0;
	int i,j;
	
	for( i = 0;i < grayImg->height;i ++)
        for( j = 0;j < grayImg->width;j ++){
  	        if(((unsigned char *)grayImg->imageData)[i * grayImg->widthStep + j]==0)
  	           count=count+1;
  	        if(((unsigned char *)grayImg->imageData)[i * grayImg->widthStep + j]==255)
  	           count2=count2+1;
        }
    //////////變成背黑前白處理參數顯示//////////    
   /* printf("\ncount:%d\n",count);
    printf("count2:%d\n",count2);*/
    //////////變成背黑前白處理//////////
    if(count2>count){
	   for( i = 0;i < grayImg->height;i ++)
           for( j = 0;j < grayImg->width;j ++){
               if(((unsigned char*)(grayImg->imageData))[i *  grayImg->widthStep + j ] == 0)
                  grayImg->imageData[i *  grayImg->widthStep + j ]=255; 
               else
                  grayImg->imageData[i *  grayImg->widthStep + j ]=0;
           }/**/
           s1=(stack *) malloc(count*sizeof(stack));
           s2=(stack *) malloc(count*sizeof(stack));
           sT=(stack *) malloc((count2+count)*sizeof(stack));
           sst=(stack *) malloc((count2+count)*sizeof(stack));
	} 
	else{
	   s1=(stack *) malloc(count*sizeof(stack));
       s2=(stack *) malloc(count*sizeof(stack));
       sT=(stack *) malloc((count2+count)*sizeof(stack));
       sst=(stack *) malloc((count2+count)*sizeof(stack));
	}
	
}

//影像裁邊(上下、左右)//
int cutparameter(IplImage * grayImg,int event) {
	int i,j;
	int state=0;
	int statew=0,stateb=0,leftcutnum=0,tcutnum=0,tcutmax=0,whitenum=0,dcutmax=0,dcutnum=0,rightcutnum=0;
	if(event==1){
	cutleft=(cutinf *) malloc(grayImg->width*sizeof(cutinf));
	
	   for( i = 0;i < grayImg->width;i ++){
	    	cutleft[i].bwcount=0;
	    	cutleft[i].whitemax=0;
		}
	    ///搜尋cutleft線///
	for( j = 0; j<  grayImg->width;j++){
       for( i = 0; i<grayImg->height ;i ++){
       	   if(statew==1)
			 if(stateb==1)/**/{
			 	cutleft[j].bwcount++;
			 	statew=0;
			 	stateb=0;
			 	 if(cutleft[j].whitemax<whitenum){
		         	cutleft[j].whitemax=whitenum;
		         	//tcutmax=i;  	
				 }
			 	whitenum=0;	
			 }
           if(((unsigned char*) (grayImg->imageData))[i *  grayImg->widthStep + j ]==255){
           	 statew=1;
  	         if(stateb==1)
  	            stateb=0;
  	           whitenum++; 
		   }
  	       else{
		       stateb=1;         
		   }
        }
        state++;
		if(   /*cutleft[j].whitemax<=grayImg->height*0 ||*/ (cutleft[j].bwcount>=8 && cutleft[j].whitemax<=grayImg->height *0.1) || cutleft[j].whitemax>=grayImg->height *0.95 /*|| cutleft[j].whitemax>=grayImg->height *0.1*/  /*|| cuttop[i].bwcount==1*/ ){//cutleft[j].whitemax>=((double)(grayImg->height))*1 ||
        	//tcutnum=cutleft[j].whitemax;
        	//tcutmax=j;
        	leftcutnum=j;
        	//printf("\ntcutnum:%d",tcutnum);
        	
           if(cutleft[j].whitemax<=grayImg->height*0.95 && cutleft[j].bwcount<=2  ){
            	state++;
            	if(state>=2)
			      break;	
			}/* */
        	   
		}
		//if(cutleft[j].whitemax<=grayImg->height*0.90  ){
			//if(state>=2)
			     // break;
		//}
              
		
		//cutleft[j].whitemax==((double)(grayImg->height))*0 &&
			//|| (cutleft[j].whitemax>=((double)(grayImg->height))*1 && cutleft[j].whitemax==0
		if(  j>grayImg->width*1/6  /* && ( cutleft[j].whitemax<=grayImg->height*0.2 ) */ /* && (cutleft[j].whitemax>=grayImg->height*0.7 *//*|| cutleft[j].whitemax<=grayImg->height*0.1 )*/ ){//cutleft[j].bwcount==0  ||
		   	//printf("\n  i:%d",j);
		   	//printf("\n  cuttop[%d].whitemax:%d",j,cutleft[j].whitemax);
		   	//printf("\ncuttop[%d].bwcount:%d",j,cutleft[j].bwcount);
		break;
	    }	   
       // if(cutdown[i].bwcount<10)
		    //  if(cuttop[tcutmax].whitemax>((double)(grayImg->width))*2/7 || cuttop[tcutmax].whitemax!=0 ) 
		  // printf("\ncutleft[%d].whitemax:%d",j,cutleft[j].whitemax);   
		  //printf("\ncutleft[%d].bwcount:%d",j,cutleft[j].bwcount);       
        
       // if(cutdown[i].bwcount>=10 && cuttop[tcutmax].whitemax>((double)(grayImg->width))*2/7)
		//   break;
		//printf("\ncutleft[j].whitemax:%d",cutleft[j].whitemax>=((double)(grayImg->height))*1);
	}	
	//printf("\nleftcutmax:%d",leftcutnum);
	return leftcutnum;
	}
	else if(event==2){
	cutright=( cutinf *) malloc(grayImg->width*sizeof(cutinf));
	//cvWaitKey(0);
/*	  */  
for( i = 0;i < grayImg->width;i ++){
	    	cutright[i].bwcount=0;
	    	cutright[i].whitemax=0;
		}
	statew=0;
	stateb=0;
	whitenum=0;
	//tcutmax=0;
	//dcutmax=0;
	//tcutnum=0;
	dcutnum=0; 
	 rightcutnum= grayImg->width-1;
	    ///搜尋cutright線///
	for(  j= grayImg->width-1 ;j >0;j --){
       for( i =0 ;i < grayImg->height;i ++){
       	   if(statew==1)
			 if(stateb==1){
			 	cutright[j].bwcount++;
			 	statew=0;
			 	stateb=0;
			 	 if(cutright[j].whitemax<whitenum){
		         	cutright[j].whitemax=whitenum;
		         	//tcutmax=i;  	
				 }
			 	whitenum=0;	
			 }
           if(((unsigned char*) (grayImg->imageData))[i *  grayImg->widthStep + j ]==255){
           	 statew=1;
  	         if(stateb==1)
  	            stateb=0;
  	           whitenum++; 
		   }
  	       else{
		       stateb=1;         
		   }
        }
		if(   /*(cutright[j].whitemax==0)|| */ (cutright[j].bwcount>9 && cutright[j].whitemax<=grayImg->height *0.1 ) || cutright[j].whitemax>=grayImg->height *0.95 /*|| cutleft[j].whitemax<=grayImg->height *0.1*//*|| cuttop[i].bwcount==1*/){//cutright[j].whitemax>=((double)(grayImg->height))*1 ||
        	//tcutnum=cutright[j].whitemax;
        	//tcutmax=j;
        	rightcutnum=j;
        	//printf("\ntcutmax:%d",tcutmax);
        	if(cutright[j].whitemax<=grayImg->height*0.95 && cutright[j].bwcount<=2 ){
        		state++;
        		if(state>=2)
        		   break;
			}
        	   /**/
		}
			
		if(  j<grayImg->width*5/6  /*&& ( ((double)cutleft[j].whitemax/grayImg->height)>=0.4 )*/ /*&& cutleft[j].whitemax>=grayImg->height*0.7*/ ){//cutright[j].bwcount==0 ||
		   //	printf("\n  i:%d",i);
		   //	printf("\n  cuttop[%d].whitemax:%d",i,cuttop[i].whitemax);
		   //	printf("\ncuttop[%d].bwcount:%d",i,cuttop[i].bwcount);
		break;
	    }	   
       // if(cutdown[i].bwcount<10)
		    //  if(cuttop[tcutmax].whitemax>((double)(grayImg->width))*2/7 || cuttop[tcutmax].whitemax!=0 ) 
		  //  printf("\ncuttop[%d].whitemax:%d",i,cuttop[i].whitemax);   
		   //printf("\ncuttop[%d].bwcount:%d",i,cuttop[i].bwcount);       
        
       // if(cutdown[i].bwcount>=10 && cuttop[tcutmax].whitemax>((double)(grayImg->width))*2/7)
		//   break;
	}	
	//printf("\nrightcutmax:%d",rightcutnum);
	return rightcutnum;
	//cvWaitKey(0);
	}/*  */ 
	else if(event==3){
	
	 statew=0,stateb=0,whitenum=0,tcutmax=0,dcutmax=0,tcutnum=0,dcutnum=0;     
	    ///裁邊///
	   
/*******************************/
/*******************************/
 
	    cuttop=(cutinf *) malloc(grayImg->height*sizeof(cutinf));
	    
	    for( i = 0;i < grayImg->height;i ++){	
	    	cuttop[i].bwcount=0;
	    	cuttop[i].whitemax=0;
		}
     ///上下裁邊///		
	///搜尋cuttop線///	
	for( i = 0;i < grayImg->height;i ++){
       for( j = 0;j < grayImg->width;j ++){
       	   if(statew==1)
			 if(stateb==1){
			 	cuttop[i].bwcount++;
			 	statew=0;
			 	stateb=0;
			 	 if(cuttop[i].whitemax<whitenum){
		         	cuttop[i].whitemax=whitenum;
		         	//tcutmax=i;  	
				 }
			 	whitenum=0;	
			 }
           if(((unsigned char*) (grayImg->imageData))[i *  grayImg->widthStep + j ]==255){
           	 statew=1;
  	         if(stateb==1)
  	            stateb=0;
  	           whitenum++; 
		   }
  	       else{
		       stateb=1;         
		   }
        }
		if( (cuttop[i].whitemax>((double)(grayImg->width))*5/7 )|| (cuttop[i].whitemax==0) || (cuttop[i].bwcount==2)  ){
        	tcutnum=cuttop[i].whitemax;
        	tcutmax=i;
        	//printf("\ntcutmax:%d",tcutmax);
		}
			
		if((cuttop[i].bwcount>=7) && (cuttop[i].whitemax<((double)(grayImg->width))*2/7)   ){
		   	//printf("\n  46i:%d",i);
		   //	printf("\n  cuttop[%d].whitemax:%d",i,cuttop[i].whitemax);
		   //	printf("\ncuttop[%d].bwcount:%d",i,cuttop[i].bwcount);
		break;
	    }	   
       // if(cutdown[i].bwcount<10)
		    //  if(cuttop[tcutmax].whitemax>((double)(grayImg->width))*2/7 || cuttop[tcutmax].whitemax!=0 ) 
		  //  printf("\ncuttop[%d].whitemax:%d",i,cuttop[i].whitemax);   
		   //printf("\ncuttop[%d].bwcount:%d",i,cuttop[i].bwcount);       
        
       // if(cutdown[i].bwcount>=10 && cuttop[tcutmax].whitemax>((double)(grayImg->width))*2/7)
		//   break;
	}	
	//printf("\ntcutmax:%d",tcutmax);
	return tcutmax;
	//cvWaitKey(0);
	}
	else if(event==4){
	statew=0,stateb=0,whitenum=0,tcutmax=0,dcutmax=0,tcutnum=0,dcutnum=0; 	
	cutdown=(cutinf *) malloc(grayImg->height*sizeof(cutinf));
	whitenum=0;
	for( i = 0;i < grayImg->height;i ++){	
	    cutdown[i].bwcount=0;
	    cutdown[i].whitemax=0;
	}	
	///搜尋cutdown線///
	dcutmax=grayImg->height-1;	
for( i =  grayImg->height-1;i >0;i --){
       for( j =0 ;j < grayImg->width;j ++){
       	   if(statew==1)
			 if(stateb==1){
			 	cutdown[i].bwcount++;
			 	statew=0;
			 	stateb=0;
			 	 if(cutdown[i].whitemax<whitenum){
		         	cutdown[i].whitemax=whitenum;
		         	//tcutmax=i;  	
				 }
			 	whitenum=0;	
			 }
           if(((unsigned char*) (grayImg->imageData))[i *  grayImg->widthStep + j ]==255){
           	 statew=1;
  	         if(stateb==1)
  	            stateb=0;
  	           whitenum++;   
		   }
  	       else{
		       stateb=1;         
		   }
        }
		if(cutdown[i].whitemax>((double)(grayImg->width))*5/7 || cutdown[i].whitemax==0 || cuttop[i].bwcount==2){
        	dcutnum=cutdown[i].whitemax;
        	dcutmax=i;
        	//printf("\ntcutmax:%d",tcutmax);
		}
		//if(dcutmax>20)	
		if(cutdown[i].bwcount>=7  &&/**/ cutdown[i].whitemax<((double)(grayImg->width))*2/7 ){
		   	//printf("\n  i:%d",i);
		   	//printf("\n  cutdown[%d].whitemax:%d",i,cutdown[i].whitemax);
		   	//printf("\ncutdown[%d].bwcount:%d",i,cutdown[i].bwcount);
		   	//dcutmax=30;
		break;
	    }	   
       // if(cutdown[i].bwcount<10)
		    //  if(cuttop[tcutmax].whitemax>((double)(grayImg->width))*2/7 || cuttop[tcutmax].whitemax!=0 ) 
		  //printf("\ncutdown[%d].whitemax:%d",i,cutdown[i].whitemax);   
		  // printf("\ncutdown[%d].bwcount:%d",i,cutdown[i].bwcount);       
        //printf("\n  i:%d",i);
       // if(cutdown[i].bwcount>=10 && cuttop[tcutmax].whitemax>((double)(grayImg->width))*2/7)
		//   break;
	}
	//printf("\ntcutmax:%d",tcutmax);	
	//printf("\ndcutmax:%d",dcutmax);
	return dcutmax;
	}
	else
	  return -1;

}
//裁邊影像(上下、左右)//
void cutimg(IplImage *cutImg,IplImage *cuttest,IplImage *grayImg,int tcutmax,int dcutmax,int  leftcutnum,int rightcutnum) {
	//cutImg = cvCreateImage(cvSize(colorImg->width,dcutmax-tcutmax),8,1);
	//cuttest=cvCreateImage(cvSize(colorImg->width,dcutmax-tcutmax),8,1);
	//cutImg = cvCreateImage(cvSize(rightcutnum-leftcutnum,dcutmax-tcutmax),8,1);
	//cuttest=cvCreateImage(cvSize(rightcutnum-leftcutnum,dcutmax-tcutmax),8,1);
	int i,j;
	for(j=0;j<cutImg->height;j++)
	   for(i=0;i<cutImg->width;i++){
	   	cutImg->imageData[(j) *  cutImg->widthStep + i ]=0;
	   	cuttest->imageData[(j) *  cuttest->widthStep + i ]=0;
	   }
	 
	for(j=tcutmax;j<dcutmax;j++)
	   for(i=leftcutnum;i<rightcutnum;i++){
	   	
	   cutImg->imageData[(j-tcutmax) *  cutImg->widthStep + (i-leftcutnum) ]=	grayImg->imageData[j *  grayImg->widthStep + i ];
	   cuttest->imageData[(j-tcutmax) *  cuttest->widthStep + (i-leftcutnum) ]=	grayImg->imageData[j *  grayImg->widthStep + i ];
	   }
	
}
 
///字元相鄰特徵///
void neighborchar(int numa,stack * area1,stack * area2,CvRect *ect,IplImage *cutImg){
	int j,i,event,EVENT,Condition3,Condition2,Condition,Condition_1;
	int p1_hy,p2_hy,diff_p1y_h,diff_p2y_h,p1_hx,p2_hx,diff_p1x_h,diff_p2x_h;
	int dx,dx1,dx2,dx3,dx4,refheight,h,diff_h,tempcount=0,mark=0,tempcount1=0,k,charnum=0;
  	double w1=0;
	for(j=0;j<numa;j++){
        for(i=0;i<numa;i++){
	         EVENT=0;
	         event=0;
	         Condition3=0;
	         Condition2=0;
	         Condition=0;
	         Condition_1=0;
             p1_hy=ect[j].y+ect[j].height;
             p2_hy=ect[i].y+ect[i].height;
             diff_p1y_h=p1_hy-ect[i].y;
             diff_p2y_h=p2_hy-ect[j].y;
             p1_hx=ect[j].x+ect[j].width;
             p2_hx=ect[i].x+ect[i].width;
             diff_p1x_h=ect[j].x-p2_hx;
             diff_p2x_h=ect[i].x-p1_hx;
          	  
            	  ////(1)由右到左掃描////
               ////(2)由參考之左邊小於選擇右邊(重疊)//// 
            if(ect[j].x >= p2_hx && diff_p1x_h>=0 ) {
		         dx1= ect[j].x-p2_hx;
		         event=1;
		         EVENT=1;
			 } 
			 else if( ect[j].x < p2_hx && diff_p1x_h<=0 && p1_hx > p2_hx) {
		        dx2= p2_hx-ect[j].x;
		        event=2;
		        EVENT=2;
			 }
			 
		     ////(1)由左到右掃描//// 
			 ////(2)由參考之右邊大於選擇左邊 (重疊)//// 
           	 if(p1_hx <= ect[i].x && diff_p2x_h>=0){
           	     dx3=ect[i].x-p1_hx;
           	     event=3;
           	     EVENT=3;
			 } 	
		     else if(p1_hx >= ect[i].x && diff_p2x_h<=0 && ect[j].x < ect[i].x) {
		          dx4=p1_hx-ect[i].x;
		          event=4;
		          EVENT=4;
			 }/**/
			 
             if(event==1 || event==2 || event==3 || event==4 )
                if( (( abs(dx1) <= (float)ect[j].width*0.4 ) ||( /*abs(dx2)*/ dx2<= (float)ect[j].width*0.4 ) || ( abs(dx3) <= (float)ect[j].width*0.4)|| ( /*abs(dx4)*/ dx4<= (float)ect[j].width*0.4) ) && j!=i ){/**/
                	////處理各種情況高度數學模型//// 
                	////處理介於高度的區塊////
                  if(ect[j].height > ect[i].height ){
				   
                      if(ect[j].y <= ect[i].y && p1_hy >= p2_hy){
                      	 Condition2=1;
                      	 refheight=ect[i].height;
					  }
                   }     
                   else{
                   	  if(ect[i].y <=ect[j].y && p2_hy >= p1_hy){
                      	 Condition2=2;
                      	 refheight=ect[j].height;
					  }
                   	
				   }				   
		        ////處理介於參考高度的區塊//// 
			       if(Condition2){
		       	      h=refheight;
		       	      event=5;
			       } 
			       else{
			     
			       	        ////(一)由上到下移動(選擇I)//// 
							////  (1)參考J高度>=選擇I高度////
							////  (2)參考J頂座標>選擇I頂座標////
							 //// (3)參考J頂座標<=選擇I底座標////
							////  ----(4)參考J頂座標>選擇I頂座標----//// 
							////  (二)由上到下移動(選擇I)//// 
							////  (1)參考J高度<選擇I高度////
							////  (2)參考J頂座標>選擇I頂座標////
							////  (3)參考J頂座標<=選擇I底座標 ////
							////  (4)參考J底座標>選擇I底座標////
					       
										   			    
			       	    if(ect[j].height >= ect[i].height ){
			       	       if(ect[j].y > ect[i].y) 
			       	       	  if(ect[j].y <= p2_hy){
			       	       	  	diff_h=p2_hy-ect[j].y;
			       	       	  	Condition3=1;
			       	       	  	event=6;
							  }
		                }
		                
		                else if(ect[j].height < ect[i].height){
		                        if(ect[j].y > ect[i].y) 
			       	               if( ect[j].y <= p2_hy)
			       	                  if(p1_hy > p2_hy){
			       	                  	diff_h=p2_hy-ect[j].y;
			       	                  	Condition3=2;
			       	                  	event=7;
									  }
			       	                                   
		                }
		               ////(一)由下到上移動(選擇I)//// 
						////	  (1)參考J高度>=選擇I高度////
						////	  (2)參考J頂座標<選擇I頂座標////
						////	  (3)參考J底座標>=選擇I頂座標////
						////	  (4)參考J底座標<選擇I底座標////
						////	  (二)由下到上移動(選擇I) ////
						////	  (1)參考J高度<選擇I高度////
						////	  (2)參考J頂座標<選擇I頂座標////
						////	  (3)參考J底座標>=選擇I頂座標 ////
						////	  --(4)參考J頂座標<選擇I頂座標--- ////
		                if(ect[j].height >= ect[i].height ){
			       	       if(ect[j].y < ect[i].y) 
			       	          if( p1_hy>=ect[i].y )
			       	             if(p1_hy < p2_hy){
			       	             	diff_h=p1_hy-ect[i].y;
			       	             	Condition3=1;
			       	             	event=8;
								 }        
		                }
		                 
		                else if(ect[j].height < ect[i].height){
		                        if(ect[j].y < ect[i].y) 
			       	               if( p1_hy>= ect[i].y){
			       	               	diff_h=p1_hy-ect[i].y;
			       	               	Condition3=2;
			       	               	event=9;
								   }		
		                }
		           }
			       /*************************************/ 
           	       ///辨斷高度_1//// 
           	       if(event==5){
					  
           	          if(Condition2==1) 
           	             Condition= (float)h/ect[j].height >= 0.8   &&  1>= (float)h/ect[j].height;
           	          else if(Condition2==2) 
           	            Condition= (float)h/ect[i].height >= 0.8   &&  1>= (float)h/ect[i].height;
           	       } 
           	       else
					  if(event==6 || event==7 || event==8|| event==9){
					  	if(Condition3==1) 
           	               Condition= (float)diff_h/ect[j].height >= 0.6   &&  1>= (float)diff_h/ect[j].height;
           	            else if(Condition3==2) 
           	               Condition= (float)diff_h/ect[i].height >= 0.6   &&  1>= (float)diff_h/ect[i].height;
					  }/**/
					 ///辨斷高度_2//// 
           	       if(event==5){
           	          if(Condition2==1) 
           	             Condition_1= (float)h/ect[j].height >= 0.05   &&  1>= (float)h/ect[j].height;
           	          else if(Condition2==2) 
           	            Condition_1= (float)h/ect[i].height >= 0.05   &&  1>= (float)h/ect[i].height;
           	       } 
           	       else
					  if(event==6 || event==7 || event==8|| event==9){
					  	if(Condition3==1) 
           	               Condition_1= (float)diff_h/ect[j].height >= 0.6   &&  1>= (float)diff_h/ect[j].height;
           	            else if(Condition3==2) 
           	               Condition_1= (float)diff_h/ect[i].height >= 0.6   &&  1>= (float)diff_h/ect[i].height;
					  }/**/ 
					    ////相鄰演算法////
					   if(Condition){

				    ////字元與車牌邊緣特徵////
				    //w1=(float)(ect[i].width+1)/(ect[i].height+1);////車牌比例 
				    if(EVENT==1 || EVENT==2 || EVENT==3 || EVENT==4 )
				  // if(  w1>0.20 && w1<0.62) 
				    if(ect[j].height>0.39*cutImg->height && ect[i].height>0.39*cutImg->height)/**/
				    if(ect[j].width<0.2*cutImg->width && ect[i].width<0.2*cutImg->width ) {
				  	////全部相鄰特徵區塊////
				  	if(area2[j].label==0 ){
				       	 area2[j].x=ect[j].x;
				         area2[j].y=ect[j].y;
				         area2[j].height=ect[j].height;
				         area2[j].width=ect[j].width;
				         tempcount++;
				         charnum++;
				         area2[j].label+=tempcount;
				         area2[j].state=1;///
				         area2[i].label+=tempcount;
				         area2[i].state=1;
					   }
					   else if(area2[j].label!=0 ){
					   	 area2[j].x=ect[j].x;
				         area2[j].y=ect[j].y;
				         area2[j].height=ect[j].height;
				         area2[j].width=ect[j].width;
				         area2[j].state=1;
				         area2[i].label=area2[j].label;
				         area2[i].x=ect[i].x;
				         area2[i].y=ect[i].y;
				         area2[i].height=ect[i].height;
				         area2[i].width=ect[i].width;
				         area2[i].state=1;
				         charnum++;
					   }
				 }
			}
				if(Condition_1){
				  	////全部相鄰特徵區塊////
				   if(ect[i].width<0.2*cutImg->width ){
				  /////////////////複製area2 資料/////////////////////
				  		if( area2[j].state==1){
				         	area1[j].state=1;
				         	area1[j].index=j;
				         	area1[j].x=ect[j].x;
				            area1[j].y=ect[j].y;
				            area1[j].height=ect[j].height;
				            area1[j].width=ect[j].width;
				         	if( area2[i].state==1){
				         		area1[i].x=ect[i].x;
				                area1[i].y=ect[i].y;
				                area1[i].height=ect[i].height;
				                area1[i].width=ect[i].width;
				         		area1[i].state=1;
				         	    area1[i].index=i;
							 }
						 }
						
				/////////////////area2 資料加入破折號和相鄰/////////////////////
				  		if(area2[j].state==1 && area1[i].state!=1 ){
				  			if(EVENT==1 || EVENT==3 ){
				  				area1[j].x=ect[j].x;
						        area1[i].x=ect[i].x;
								area1[j].y=ect[j].y;
						        area1[i].y=ect[i].y;
								area1[j].width=ect[j].width;
						        area1[i].width=ect[i].width;
	                           if(EVENT==1)
				  			     dx= area2[j].x-(area1[i].x+area1[i].width);
				  			   else if(EVENT==3)
								 dx= area1[i].x-(area2[j].x+area2[j].width);  
				  	        if(dx<=area2[j].width*0.25)
						        if(event==5 ){
				  				   area1[i].x=ect[i].x;
				                   area1[i].y=ect[i].y;
				                   area1[i].height=ect[i].height;
				                   area1[i].width=ect[i].width;
				  			   	   area1[i].state=1;
							    }
						   }
						}/**/
					}
				}
			}
		}	
   }
   return ;
}
//}
//找破折號位置//
int  finddashchar(int numa,stack * area1,IplImage *cutImg){
	int j,i,event,EVENT1,Condition3,Condition2,Condition,Condition_1;
	int p1_hy,p2_hy,diff_p1y_h,diff_p2y_h,p1_hx,p2_hx,diff_p1x_h,diff_p2x_h;
	int dx,refheight,h,diff_h,tempcount=0,mark=0,tempcount1=0,symbolstate=0;
	int Symbol=0,breakstop=0;
	int EVENT2=0;
	int dx1,dx2,dx3,dx4;
	int count=0,k=0,temp=0,temp2,temp3,temp4;
	
for(k=0;k<2;k++){
	if(k==1)
	  symbolstate=0;
	for(j=0;j<numa;j++){
		 if((area1[j].height>0.4*cutImg->height) && area1[j].state==1) 
			 symbolstate++;
  for(i=0;i<numa;i++){///
  	  if(area1[j].state==1 && area1[i].state==1/*1*/){
		  /////////////相鄰判斷/////////////////
	         event=0;
	         EVENT1=0;
	         EVENT2=0;
	         Condition3=0;
	         Condition2=0;
	         Condition=0;
	         Condition_1=0;
	        
             p1_hy=area1[j].y+area1[j].height;
             p2_hy=area1[i].y+area1[i].height;
             diff_p1y_h=p1_hy-area1[i].y;
             diff_p2y_h=p2_hy-area1[j].y;
           
             p1_hx=area1[j].x+area1[j].width;
             p2_hx=area1[i].x+area1[i].width;
             diff_p1x_h=area1[j].x-p2_hx;
             diff_p2x_h=area1[i].x-p1_hx;
          	 
			   
             //(1)由右到左掃描
            //(2)由參考之左邊小於選擇右邊(重疊)
             if(area1[j].x >= p2_hx && diff_p1x_h>=0 ) {
		         dx= area1[j].x-p2_hx;
		         dx1=dx;
		         event=1;
		         EVENT1=1;
			 } 
			 
			 else if( area1[j].x < p2_hx && diff_p1x_h<=0 && p1_hx > p2_hx) {
		        dx= area1[j].x-p2_hx;
		        dx2=dx;
		        event=2;
		        EVENT1=2;
			 }
			}
			  
			 
			 
		     //(1)由左到右掃描 
			//(2)由參考之右邊大於選擇左邊 (重疊) 
            	 if(p1_hx <= area1[i].x && diff_p2x_h>=0){
           	     dx=area1[i].x-p1_hx;
           	     dx3=dx;
           	     event=3;
           	    EVENT2=1;
			 } 	
		     else if(p1_hx >= area1[i].x && diff_p2x_h<=0 && area1[j].x < area1[i].x) {
		          dx=area1[i].x-p1_hx;
		          dx4=dx;
		          event=4;
		          EVENT2=2;
			 } 
		
			
             if(event==1 || event==2 || event==3 || event==4 )
                if( ( /*abs(dx) <= (float)area1[j].width*0.25 +1*/ (dx1 <= (float)area1[j].width*0.2 +1) || (dx2 <= (float)area1[j].width*0.2 +1) || (dx3 <= (float)area1[j].width*0.2 +1)|| (dx4 <= (float)area1[j].width*0.2 +1))  && j!=i ){/**/
                	///處理各種情況高度數學模型 
                	//處理介於高度的區塊
                  if(area1[j].height > area1[i].height ){
				   
                      if(area1[j].y <= area1[i].y && p1_hy >= p2_hy){
                      	 Condition2=1;
                      	 refheight=area1[i].height;
					  }
                   }     
                  /* else{
                   	  if(area1[i].y <=area1[j].y && p2_hy >= p1_hy){
                      	 Condition2=2;
                      	 refheight=area1[j].height;
					  }
                   	
				   }*/
                    				   
		        //處理介於參考高度的區塊
			       if(Condition2){
		       	      h=refheight;
		       	      event=5;
			       } 
			       else{
			     
			       	        //(一)由上到下移動(選擇I) 
							  //(1)參考J高度>=選擇I高度
							  //(2)參考J頂座標>選擇I頂座標
							  //(3)參考J頂座標<=選擇I底座標
							  //----(4)參考J頂座標>選擇I頂座標---- 
							  //(二)由上到下移動(選擇I) 
							  //(1)參考J高度<選擇I高度
							  //(2)參考J頂座標>選擇I頂座標
							  //(3)參考J頂座標<=選擇I底座標 
							  //(4)參考J底座標>選擇I底座標
					        
										   			    
			       	    if(area1[j].height >= area1[i].height ){
			       	       if(area1[j].y > area1[i].y) 
			       	       	  if(area1[j].y <= p2_hy){
			       	       	  	diff_h=p2_hy-area1[j].y;
			       	       	  	Condition3=1;
			       	       	  	event=6;
							  }
			       	          	 
			       	          	
						          
		                }
		                
		                else if(area1[j].height < area1[i].height){
		                        if(area1[j].y > area1[i].y) 
			       	               if( area1[j].y <= p2_hy)
			       	                  if(p1_hy > p2_hy){
			       	                  	diff_h=p2_hy-area1[j].y;
			       	                  	Condition3=2;
			       	                  	event=7;
									  }
			       	                                   
		                }
		                
		                   // (一)由下到上移動(選擇I) 
							 // (1)參考J高度>=選擇I高度
							 // (2)參考J頂座標<選擇I頂座標
							 // (3)參考J底座標>=選擇I頂座標
							 // (4)參考J底座標<選擇I底座標
							 // (二)由下到上移動(選擇I) 
							 // (1)參考J高度<選擇I高度
							 // (2)參考J頂座標<選擇I頂座標
							 // (3)參考J底座標>=選擇I頂座標 
							 // --(4)參考J頂座標<選擇I頂座標--- 
							  
					    
					                 
		                if(area1[j].height >= area1[i].height ){
			       	       if(area1[j].y < area1[i].y) 
			       	          if( p1_hy>=area1[i].y )
			       	             if(p1_hy < p2_hy){
			       	             	diff_h=p1_hy-area1[i].y;
			       	             	Condition3=1;
			       	             	event=8;
								 }
									 
						           
		                }
		                 
		                else if(area1[j].height < area1[i].height){
		                        if(area1[j].y < area1[i].y) 
			       	               if( p1_hy>= area1[i].y){
			       	               	diff_h=p1_hy-area1[i].y;
			       	               	Condition3=2;
			       	               	event=9;
								   }		
		                }
		           }
			       ///*************結束 ************************ 
			      
           	       ///辨斷高度_1 
           	       if(event==5){
					  
           	          if(Condition2==1) 
           	             Condition= (float)h/area1[j].height <= 0.2   &&  0.05<= (float)h/area1[j].height;
           	          else if(Condition2==2) 
           	            Condition= (float)h/area1[i].height <= 0.2   &&  0.05<= (float)h/area1[i].height;
           	       } 
           	       else
					  if(event==6 || event==7 || event==8|| event==9){
					  	if(Condition3==1) 
           	               Condition= (float)diff_h/area1[j].height >= 0.95   &&  1>= (float)diff_h/area1[j].height;
           	            else if(Condition3==2) 
           	               Condition= (float)diff_h/area1[i].height >= 0.95   &&  1>= (float)diff_h/area1[i].height;
					  }
					  //if(j!=0)   
					   /* if(area1[j].height>0.4*cutImg->height)//----計數錯誤 
					      symbolstate++;*/ 
				     //printf("symbolstate:%d\n",area1[j].index);/**/
					if(symbolstate>1)
					   if(Condition){
				      ///////////////////尋找破折號演算法///////////////////
				        //if(area1[j].height*0.2<area1[i].height)
				       //if(i>30)
				       // if(EVENT1==1  ||/**/ EVENT2==1 || EVENT1==2  ||  EVENT2==2/**//**/ ){
				          if((EVENT2==1 || EVENT2==2/**/) ){
				          	/*if(event==5)
				               count++;*/
				            if(k==0)   
				          	if(event==5){
				          	    Symbol=i;
				          	   // printf("symbolstate1:%d\n",symbolstate);
				          	    
				          	breakstop=1;
						       break;
						    } 
						  }
						  if((EVENT1==1 || EVENT1==2) && symbolstate>2/**/){
						   if(k==0)
						  	if(event==5){
				          	    Symbol=i;
				          	   // printf("symbolstate2:%d\n",symbolstate);
				          	breakstop=1;
						       break;
						    }   
						  }/**/
				       // }
						  
                       }
			     
				  } 

               }
               
            ///////////////////停止掃描/////////////////// 
  if(breakstop==1){
  	break;
  }   
}
}
return Symbol;
}
//讀取字元相鄰特徵//
 int readcharfeature(stack *temparea,int symbolcount,stack *sT,IplImage *cuttest,int platecontour,int platecontour1){
int counts1=0;
int event=3;
int EVENT=0; 
int Condition2=0;
int Condition3=0;
int k=0,j,i;
 int p1_hy,p2_hy,diff_p1y_h,diff_p2y_h,p1_hx,p2_hx,diff_p1x_h,diff_p2x_h;
	int dx,refheight,h,diff_h,tempcount=0,mark=0,tempcount1=0,symbolstate=0;
 int initx_1=0,inity_1=0;
    int initw_1=0,inith_1=0;
    int initx_2=0,inity_2=0;
    int initw_2=0,inith_2=0;
    int initx_3=0,inity_3=0;
    int initw_3=0,inith_3=0;
 
for(k=platecontour;k<symbolcount;k++) {
	//printf("\n\nk:%d\n",k);
	//printf("\n\neventk:%d\n",event);
	if(k<symbolcount-1){
	   	
	   p1_hy=temparea[k].y+temparea[k].height;
       p2_hy=temparea[k+1].y+temparea[k+1].height;
       diff_p1y_h=p1_hy-temparea[k+1].y;
       diff_p2y_h=p2_hy-temparea[k].y;
       p1_hx=temparea[k].x+temparea[k].width;
       p2_hx=temparea[k+1].x+temparea[k+1].width;
       diff_p1x_h=temparea[k].x-p2_hx;
       diff_p2x_h=temparea[k+1].x-p1_hx;	
	///判斷是否重疊或非重疊///
	   if(p1_hx <= temparea[k+1].x && diff_p2x_h>=0){
          dx=temparea[k+1].x-p1_hx;
          event=3;  
		   //printf("\n\nevent3:%d\n",event);   	    
	   } 	
	   else if(p1_hx >= temparea[k+1].x  &&  diff_p2x_h<=0 && temparea[k].x < temparea[k+1].x) {
	   	
	   	/**********************************************/
	   		///處理各種情況高度數學模型/// 
                	///處理介於高度的區塊///
                  if(temparea[k].height > temparea[k+1].height ){
				   
                      if(temparea[k].y <= temparea[k+1].y && p1_hy >= p2_hy){
                      	 Condition2=1;
                      	 EVENT=1;
                      	 //refheight=ect[i].height;
					  }
                   }     
                   else{
                   	  if(temparea[k+1].y <=temparea[k].y && p2_hy >= p1_hy){
                      	 Condition2=2;
                      	 EVENT=2;
                      	 //refheight=ect[j].height;
					  }
                   	
				   }
                     				   
		        ///處理介於參考高度的區塊///
			       if(Condition2){
		       	      //h=refheight;
		       	      event=5;
			       } 
			       else{
			     
			       	        //(一)由上到下移動(選擇I) 
							  //(1)參考J高度>=選擇I高度
							  //(2)參考J頂座標>選擇I頂座標
							  //(3)參考J頂座標<=選擇I底座標
							  //----(4)參考J頂座標>選擇I頂座標---- 
							  //(二)由上到下移動(選擇I) 
							  //(1)參考J高度<選擇I高度
							  //(2)參考J頂座標>選擇I頂座標
							  //(3)參考J頂座標<=選擇I底座標 
							  //(4)參考J底座標>選擇I底座標
					        
										   			    
			       	    if(temparea[k].height >= temparea[k+1].height ){
			       	       if(temparea[k].y > temparea[k+1].y) 
			       	       	  if(temparea[k].y <= p2_hy){
			       	       	  //	diff_h=p2_hy-ect[j].y;
			       	       	  	Condition3=1;
			       	       	  	event=6;
							  }
			       	          	 
			       	          	
						          
		                }
		                
		                else if(temparea[k].height < temparea[k+1].height){
		                        if(temparea[k].y > temparea[k+1].y) 
			       	               if( temparea[k].y <= p2_hy)
			       	                  if(p1_hy > p2_hy){
			       	                  //	diff_h=p2_hy-ect[j].y;
			       	                  	Condition3=2;
			       	                  	event=7;
									  }
			       	                                   
		                }
		                
		                   //(一)由下到上移動(選擇I) 
							 // (1)參考J高度>=選擇I高度
							  //(2)參考J頂座標<選擇I頂座標
							  //(3)參考J底座標>=選擇I頂座標
							  //(4)參考J底座標<選擇I底座標
							  //(二)由下到上移動(選擇I) 
							  //(1)參考J高度<選擇I高度
							  //(2)參考J頂座標<選擇I頂座標
							  //(3)參考J底座標>=選擇I頂座標 
							  //--(4)參考J頂座標<選擇I頂座標--- 
							  
					    
					                 
		                if(temparea[k].height >= temparea[k+1].height ){
			       	       if(temparea[k].y < temparea[k+1].y) 
			       	          if( p1_hy>=temparea[k+1].y )
			       	             if(p1_hy < p2_hy){
			       	             	//diff_h=p1_hy-ect[i].y;
			       	             	Condition3=1;
			       	             	event=8;
								 }
									 
						           
		                }
		                 
		                else if(temparea[k].height < temparea[k+1].height){
		                        if(temparea[k].y < temparea[k+1].y) 
			       	               if( p1_hy>= temparea[k+1].y){
			       	               	//diff_h=p1_hy-ect[i].y;
			       	               	Condition3=2;
			       	               	event=9;
								   }		
		                }
		           }
	   	/**********************************************/
	   	//printf("\n\nevent4:%d\n",event);
	   	//printf("\n\nEVENT:%d\n",EVENT);
	   	////////////判斷相鄰狀態////////////
	   	if(event==5 && EVENT==2){ ///
	   		dx=p1_hx-temparea[k+1].x; 
			initx_1=temparea[k].x;
			inity_1=temparea[k].y;
			initw_1=temparea[k].width-dx;
			inith_1=temparea[k].height;
			initx_2=temparea[k+1].x;
			inity_2=temparea[k+1].y;
			initw_2=dx;
			inith_2=temparea[k+1].height;
			initx_3=initx_2+dx;
			inity_3=temparea[k+1].y;
			initw_3=temparea[k+1].width-dx;
			inith_3=temparea[k+1].height;	
		}
	   	
	   	if(event==5 && EVENT==1){ ///
	   		dx=p1_hx-temparea[k+1].x; 
			initx_1=temparea[k].x;
			inity_1=temparea[k].y;
			initw_1=temparea[k].width-dx;
			inith_1=temparea[k].height;
			initx_2=temparea[k+1].x;
			inity_2=temparea[k].y;
			initw_2=dx;
			inith_2=temparea[k].height;
			initx_3=initx_2+dx;
			inity_3=temparea[k+1].y;
			initw_3=temparea[k+1].width-dx;
			inith_3=temparea[k+1].height;	
		}
	   	
	   	if(event==6 || event==7){ ///
	   		dx=p1_hx-temparea[k+1].x; 
			initx_1=temparea[k].x;
			inity_1=temparea[k].y;
			initw_1=temparea[k].width-dx;
			inith_1=temparea[k].height;
			initx_2=temparea[k+1].x;
			inity_2=temparea[k+1].y;
			initw_2=dx;
			inith_2=(temparea[k].y+temparea[k].height)-inity_2;
			initx_3=initx_2+dx;
			inity_3=temparea[k+1].y;
			initw_3=temparea[k+1].width-dx;
			inith_3=temparea[k+1].height;	
		}
		
		if(event==8 || event==9){ ///
	   		dx=p1_hx-temparea[k+1].x; 
			initx_1=temparea[k].x;
			inity_1=temparea[k].y;
			initw_1=temparea[k].width-dx;
			inith_1=temparea[k].height;
			initx_2=temparea[k+1].x;
			inity_2=temparea[k].y;
			initw_2=dx;
			inith_2=(temparea[k+1].y+temparea[k+1].height)-inity_2;
			initx_3=initx_2+dx;
			inity_3=temparea[k+1].y;
			initw_3=temparea[k+1].width-dx;
			inith_3=temparea[k+1].height;	
		}  
		    
		event=4;
		///////////////////////////////////////    
		    
	   }  
	}
	
//	printf("\n\nevent4q:%d\n",event);
	////////////字元掃描////////////
	if(event==3){
		for( i = 0;i < temparea[k].height;i ++)
            for( j =0;j < temparea[k].width;j ++){
                if(((unsigned char*)(cuttest->imageData))[(i+temparea[k].y) *  cuttest->widthStep + (j+temparea[k].x) ] == 255){
          	        counts1++;
          	        sT[counts1-1].x=j+temparea[k].x;
          	        sT[counts1-1].y=i+temparea[k].y;
		         }    
		    }
    }
	else if(event==4){
			for( i = inity_1;i < inith_1+inity_1;i ++)
                for( j = initx_1;j < initw_1+initx_1;j ++){
                    if(((unsigned char*)(cuttest->imageData))[i *  cuttest->widthStep + j ] == 255){
          	           counts1++;
          	           sT[counts1-1].x=j;
          	           sT[counts1-1].y=i;
		            }   
	         	} 
			for( i = inity_2;i < inith_2+inity_2;i ++)
                for( j = initx_2;j < initw_2+initx_2;j ++){
                    if(((unsigned char*)(cuttest->imageData))[i *  cuttest->widthStep + j ] == 255){
          	           counts1++;
          	           sT[counts1-1].x=j;
          	           sT[counts1-1].y=i;
		            }   
	         	}	 
			for( i = inity_3;i < inith_3+inity_3;i ++)
                for( j = initx_3;j < initw_3+initx_3;j ++){
                    if(((unsigned char*)(cuttest->imageData))[i *  cuttest->widthStep + j ] == 255){
          	           counts1++;
          	           sT[counts1-1].x=j;
          	           sT[counts1-1].y=i;
		            }   
	         	}
	        
	 	       k+=1;
	     		 	 	  		  	
	}
	event=3;
	
} 
 return counts1;	
}

//K-mean演算法//
double kmeanslop(int counts1,IplImage *cuttemp2,stack *sT,stack *s1,stack *s2,int event){
	double atan1=0;
	unsigned int d1=0,d2=0,min1=0,min2=0;
	int num1=0,num2=0,sum1=0,sum2=0;
	int i=0,j=0,num=1;
	if(event==1){
		u[0].x=cuttemp2->width/6;
	    u[0].y=cuttemp2->height/2;
	    u[1].x=cuttemp2->width/6*5;
	    u[1].y=cuttemp2->height/2;
	}
	else if(event==2){
	u[0].x=0;
	u[0].y=cuttemp2->height*1/4;
	u[1].x=0;
	u[1].y=cuttemp2->height*3/4;
		
	}
    //d1=pow((sT[i].x-u[0].x),2)+pow((sT[i].y-u[0].y),2);
	//d2=pow((sT[i].x-u[1].x),2)+pow((sT[i].y-u[1].y),2);
	num1=0;
	num2=0;
	sum1=0;
	sum2=0;
	//****類別平均初始化(結束))****//
	////k-mean演算法(code)//// 
	while(1){
		for(i=0;i<counts1;i++){
		   	d1=pow((sT[i].x-u[0].x),2)+pow((sT[i].y-u[0].y),2);
		   	d2=pow((sT[i].x-u[1].x),2)+pow((sT[i].y-u[1].y),2);
		   	if(d1<d2){
		   	   num1++;
		   	   s1[num1-1].x=sT[i].x;
			   s1[num1-1].y=sT[i].y;   	
			}
			else{
			  num2++;	
			  s2[num2-1].x=sT[i].x;
			  s2[num2-1].y=sT[i].y; 	
			}
		}		
		for(i=0;i<num1;i++){
			sum1=sum1+s1[i].x;
		   	sum2=sum2+s1[i].y;
		}
		u[0].x=(double)sum1/num1; 
		u[0].y=(double)sum2/num1;
		sum1=0;
		sum2=0;
		for(i=0;i<num2;i++){
			sum1=sum1+s2[i].x;
		   	sum2=sum2+s2[i].y;
		}
		u[1].x=(double)sum1/num2; 
		u[1].y=(double)sum2/num2;
		sum1=0;
		sum2=0;
		num1=0;
		num2=0;	
		if(num!=1){
		  if(min1>d1 || min2>d2){
		   	min1=d1;
		   	min2=d2;		   		
	      }
	      else
			break;   	
	    }
		else{
		  num=0;
		  min1=d1;
		  min2=d2;
		}
	}
	atan1=atan((double)(u[1].y-u[0].y)/(u[1].x-u[0].x))*180/PI;	
	return atan1;
} 

//水平校正參數資訊/
stack hcorrectinf(stack * xmatrix,int counts1 ,int infevent){
	/*******水平校正之顯示(cvShowImage)所需參數 ***************/
	int xmax,xmin,ymin,ymax,width,height,i,deltx,delty;
	stack infm;
	xmax=xmatrix[0].x;
	xmin=xmatrix[0].x;
	ymin=xmatrix[0].y;
	ymax=xmatrix[0].y;       
	/****取最大最小值****/
	for(i=0;i<counts1;i++){
	    if(xmax<xmatrix[i].x)
	      xmax=xmatrix[i].x;
	    if(xmin>xmatrix[i].x)
	      xmin=xmatrix[i].x;
	    if(ymax<xmatrix[i].y)
		  ymax=xmatrix[i].y; 
		if(ymin>xmatrix[i].y)
		  ymin=xmatrix[i].y;     
	}

/*	printf("\n xmax:%d\n",xmax);
	printf("\n xmin:%d\n",xmin);
	printf("\n ymax:%d\n",ymax);
	printf("\n ymin:%d\n",ymin);*/
	/************/
	////////////校正後適當空間計算_1//////
	
	if(xmin<0 ){
	    width=xmax-xmin+1;
	    deltx=-xmin;
	} 
	else{
	    width=xmax+xmin+1;
	    deltx=xmin;
	}   
	if(ymin<0 ){
	  height=ymax-ymin+1;
	  delty=-ymin;
	}   
	else{
	  height=ymax+ymin+1;
	  delty=ymin;	
	}
	
	infm.x=deltx;
	infm.y=delty;
	infm.width=width;
	infm.height=height;
	
	return infm;

}


//取得對應類別最大值// 
int getmaxvalclass(double outresult[]){
	double max=0;
	int i,index=0;
	for(i=0;i<34;i++)
		if(outresult[i]>max){
           index=i;
		   max=outresult[i];  
	    }
	//printf("max:%lf",max);    
    return index;
}
//取得輸出特徵值// 
void getplatevect(double platevect[],int index,int n){
	platevect[n]=index;
}
//誤判處理// 
void improveprocess(double platevect[],engdashnum platecharstate,int n){
	int i,j;
	//可能誤判類別//
	//B<=>11 與 8<=>8 //
	//O<=>24 與 0<=>0 //
	//I<=>18 與 1<=>1 //
	//Z<=>35 與 2<=>2 //
    for(i=0;i<n;i++){
	    if(platecharstate.label==1){//左英右數 
	  		if(platecharstate.englishnum>i){
	  			if(platevect[i]==8){
	  			   platevect[i]=11;
				}
				else if(platevect[i]==0){
				  platevect[i]=24;	
				}
				else if(platevect[i]==1){
				  platevect[i]=18;	
				}
				else if(platevect[i]==2){
				  platevect[i]=35;	
				}
			}
			else{
				if(platevect[i]==11){
	  			  platevect[i]=8;
				}
				else if(platevect[i]==24){
				  platevect[i]=0;	
				}
				else if(platevect[i]==18){
				  platevect[i]=1;	
				}
				else if(platevect[i]==35){
				  platevect[i]=2;	
				}
			}
	    }
	    else if(platecharstate.label==2){//左數右英
	  	    if(platecharstate.numbernum>i){
	  		   if(platevect[i]==11){
	  			  platevect[i]=8;
			   }
			   else if(platevect[i]==24){
				  platevect[i]=0;	
			   }
			   else if(platevect[i]==18){
				  platevect[i]=1;	
			   }
			   else if(platevect[i]==35){
				  platevect[i]=2;	
			   }
		    }
		    else{
			   if(platevect[i]==8){
	  			  platevect[i]=11;
			   }
			   else if(platevect[i]==0){
			 	  platevect[i]=24;	
			   }
			   else if(platevect[i]==1){
				  platevect[i]=18;	
			   }
			   else if(platevect[i]==2){
				  platevect[i]=35;	
			   }
		    }	
	    }
    }	
}
//改善輸出結果(破折號)// 
void improveoutput(double platevect[],int dashplace,int n){
	int numbernum=0;
	int englishnum=0;
	int engcounter=0;
	int numbcounter=0;
	int i;
	engdashnum state;
	numbernum=n-dashplace+1;
	englishnum=dashplace-1;
	 if(n==7){
	 	if(numbernum > englishnum){
	 		platecharstate.numbernum=numbernum;
	 		platecharstate.englishnum=englishnum;
	 		platecharstate.label=1;
		}
		else if(numbernum < englishnum){
			platecharstate.numbernum=englishnum;
	 		platecharstate.englishnum=numbernum;
	 		platecharstate.label=2;
		}
		//可能誤判類別//
			//B<=>11 與 8<=>8 //
			//O<=>24 與 0<=>0 //
			//I<=>18 與 1<=>1 //
			//Z<=>35 與 2<=>3 //
		improveprocess(platevect,platecharstate,n);	
	 }
	 else if(n==6){
	 	if(dashplace==4){
	 		//保持預設// 
	 		//搜尋數字(0~9) or 搜尋英文(10~35)//
	 		//次數1次不符合可能誤判字元//
			//如果為0次，則用預設值  
	 		//可能誤判類別//
			//B<=>11 與 8<=>8 //
			//O<=>24 與 0<=>0 //
			//I<=>18 與 1<=>1 //
			//Z<=>35 與 2<=>3 //
			for(i=0;i<n;i++){
			   if(i<dashplace){
			      if(platevect[i]!=11)
	  	             if(platevect[i]!=24)
			            if(platevect[i]!=18)
			              if(platevect[i]!=35)
						     if(platevect[i]>=10 && platevect[i]<36){
				  	           engcounter++;
			                 }  
			    }
				else {
				   if(platevect[i]!=8 )
	  	             if(platevect[i]!=0)
			            if(platevect[i]!=1)
			              if(platevect[i]!=2)
						     if(platevect[i]>=0 && platevect[i]<10){
				  	            numbcounter++;
			                 }  	
				} 
			} 
			//判別處理
			if(numbcounter >0 && engcounter >0){
				state.label=1;
				state.numbernum=3;
				state.englishnum=3;
			}   
			else{
				state.numbernum=3;
				state.englishnum=3;
				state.label=2;
			} 
			improveprocess(platevect,state,n);     	   
		}
		else if(numbernum > englishnum){
			platecharstate.numbernum=numbernum;
	 		platecharstate.englishnum=englishnum;
	 		platecharstate.label=1;
		}
	 	else if(numbernum < englishnum){
			platecharstate.numbernum=englishnum;
	 		platecharstate.englishnum=numbernum;
	 		platecharstate.label=2;
		}
		//可能誤判類別//
			//B<=>11 與 8<=>8 //
			//O<=>24 與 0<=>0 //
			//I<=>18 與 1<=>1 //
			//Z<=>35 與 2<=>3 //
		improveprocess(platevect,platecharstate,n);		
	 }
}
//校正輸出樣本 
void correctsample(double platevect[],int counter){
	int i;
	for(i=0;i<counter;i++){//校正輸出樣本 
	if(platevect[i]>=18 && platevect[i]<23)
	   platevect[i]+=1;    
	else if(platevect[i]>=23 && platevect[i]< 36)
	   platevect[i]+=2;  
    }
}

//輸出該類別字元結果// 
void outputchar(int dashplace,int index,int n/*, classsval[]*/){
	   //index=n;//測試結果功能 
	if(dashplace==n)
	   printf("-");     
	if(index==0)
	   printf("0");
	else if(index==1)
	   printf("1");
	else if(index==2)
	   printf("2");         
	else if(index==3)
	   printf("3");
	else if(index==4)
	   printf("4");
	else if(index==5)
	   printf("5");
	else if(index==6)
	   printf("6");
	else if(index==7)
	   printf("7");
	else if(index==8)
	   printf("8");
	else if(index==9)
	   printf("9");
	else if(index==10)
	   printf("A");
	else if(index==11)
	   printf("B");
	else if(index==12)
	   printf("C");
	else if(index==13)
	   printf("D");         
	else if(index==14)
	   printf("E");
	else if(index==15)
	   printf("F");
	else if(index==16)
	   printf("G");
	else if(index==17)
	   printf("H");
	else if(index==18)
	   printf("I");
	else if(index==19)
	   printf("J");
	else if(index==20)
	   printf("K");
	else if(index==21)
	   printf("L");
	else if(index==22)
	   printf("M");
	else if(index==23)
	   printf("N");
	else if(index==24)
	   printf("O");         
	else if(index==25)
	   printf("P");
	else if(index==26)
	   printf("Q");
	else if(index==27)
	   printf("R");
	else if(index==28)
	   printf("S");
	else if(index==29)
	   printf("T");
	else if(index==30)
	   printf("U");
	else if(index==31)
	   printf("V");
	else if(index==32)
	   printf("W");
	else if(index==33)
	   printf("X");
	else if(index==34)
	   printf("Y");
	else if(index==35)
	   printf("Z"); 
	      
}

void recognitionstep(int charnum,int n1,int n,int dashplace,double platevect[7],IplImage * img){

IplImage *src, *temp1, *ftmp,*testimg,*colorImg;
int iwidth;
int iheight;
int i,j;
int index=0;
double min_val;
double max_val;
double outresult[33];
char char1[30]="樣本匹配訓練/";
char char2[30];
CvPoint min_loc;
CvPoint max_loc;
int counter=0;// 


/*********************樣本匹配*******************************/    
    //讀待測樣本 
    //正規化處理 
    testimg = cvCreateImage(cvSize(20,30),8,1);
    //printf("width:%d ; height:%d \n",testimg->width,testimg->height);
	cvResize(img,testimg,CV_INTER_LINEAR);
    //cvShowImage("src",testimg);
    //printf("width:%d ; height:%d\n",testimg->width,testimg->height);
    //讀標準樣本 
    for(i=0;i<34;i++){//第掃描0~Z字元與測試字元匹配 
    sprintf(char2, "%d",i);
    //strcat(char1,"/");
    strcat(char1,char2);
    //charnum=strtok(char1, ".jpg");//字串分割 
    //樣本匹配訓練
    strcat(char1,".bmp");
    //printf("%s\n",char1);
    
    if ((temp1 = cvLoadImage(char1, CV_LOAD_IMAGE_UNCHANGED)) == 0) {
       //return -1;
    }/**/
    char1[13]='\0';
    /*cvShowImage("testimg",temp1);
    cvWaitKey(0);*/
    ftmp = cvCreateImage(cvSize(1, 1),IPL_DEPTH_32F, 1);
    cvMatchTemplate(testimg, temp1, ftmp, CV_TM_CCOEFF_NORMED);
    cvMinMaxLoc(ftmp, &min_val, &max_val, &min_loc, &max_loc, NULL);
    //printf("%d: min_val:%lf\n",i,max_val);
    outresult[i]=max_val;//取得每類匹配相關系數 
    cvReleaseImage(&temp1 );
    //cvWaitKey(0);
    }
    //取得對應類別最大值//
    index=getmaxvalclass(outresult);
    //printf("%d: index:%d\n",n,index);
    //取得輸出特徵值// 
    counter++;
    
    //getplatevect(platevect,index,counter-1);
	getplatevect(platevect,index,n-1);	   
//}
/***********************結束*****************************/ 

return ;	
}

int cutcharfeature(int num,CvRect * ect_1,IplImage *vvtest,int charnum,int n1,int dashplace){
	int ii,n=0,i;
	CvSize  size1;
	double w1;
	int count,count2;
	IplImage *pDstImg = NULL; 
	CvMat *mat_roi;
	char chr[10];
	char chr1[10];
	char str[10]=".bmp";
	char char1[20];
	double platevect[7];
	 
  for(ii=0;ii<num;ii++){
   size1 = cvSize(ect_1[ii].width, ect_1[ii].height);
   //printf("\n w1:%f\n",w1);
	   count=0;
       count2=0;
       pDstImg = cvCreateImage( size1, IPL_DEPTH_8U, 1);
       mat_roi = cvCreateMat(ect_1[ii].height, ect_1[ii].width,CV_8UC1);
       cvGetSubRect(vvtest, mat_roi, ect_1[ii]);
       cvGetImage(mat_roi, pDstImg);
		//cvRectangle( vvtest, cvPoint( ect_1[ii].x, ect_1[ii].y ),cvPoint( ect_1[ii].x + ect_1[ii].width, ect_1[ii].y + ect_1[ii].height ), cvScalar(255,255,0,0), 2,2,0 );
           cvGetSubRect(vvtest, mat_roi, ect_1[ii]);
            cvGetImage(mat_roi, pDstImg);	
            n++;
           recognitionstep(charnum,n1,n,dashplace,platevect,pDstImg); 
         
           sprintf(chr, "%d", n);
           sprintf(chr1, "%d", n1);
           sprintf(char1, "%d", charnum);
           strcat(char1, "_");
           strcat(char1, chr1); 
           strcat(char1, "_");
           strcat(char1, chr);
		   strcat(char1, str); 

           cvSaveImage(char1,  pDstImg, 0 );
           cvReleaseMat(&mat_roi);//釋放資源
          // cvReleaseImage(&pDstImg);//釋放資源 
   }
   //校正輸出樣本 
   correctsample(platevect,n);
   //改善輸出結果(破折號)// 
   //improveoutput(platevect,dashplace,n);
   //輸出該類別字元結果//
   printf("車牌辨識結果:");
   for(i=0;i<n;i++){
	    //printf("platevect[%d]:%lf\n",i,platevect[i]);
        outputchar(dashplace-1,platevect[i],i);
    }/**/
    printf("\n");
   return n;
}

int getplatecharvector(int symbolcount,stack* symbolarrray,CvRect* charfeature,IplImage *cutImg,int* data){
	int charcount=0,charindex=0,platecharsig=0;
//CvRect* charfeature;
float w1;
int j,i;


//cvShowImage("HelloWorld-2",cutImg);
//cvWaitKey(0);
 //charfeature = (CvRect *) malloc(symbolcount*sizeof(CvRect));
 for(j=0;j<symbolcount;j++){
 	if(symbolarrray[j].width<0.2*cutImg->width)
 	   if(symbolarrray[j].height>0.39*cutImg->height){
 	          for(i=0;i<symbolcount;i++){
 	          	  if(symbolarrray[i].height!=0)
                    w1=(float)(symbolarrray[i].width)/(symbolarrray[i].height);////車牌比例 
                  else
                    w1=-1;
 	          	  if(  (w1>0.32 && w1<0.72) || (w1>0.15 && w1<0.3) )
 		            if((abs(symbolarrray[j].height-symbolarrray[i].height)<8)){
		               charcount++;
		            charfeature[charcount-1].x=symbolarrray[i].x;
 			        charfeature[charcount-1].y=symbolarrray[i].y;
 			        charfeature[charcount-1].width=symbolarrray[i].width;
 			        charfeature[charcount-1].height=symbolarrray[i].height;
		           }
	          }
       }
 	if((charcount)==7 || (charcount)==6){
 		charindex=j;
 		platecharsig=1;
 	//	printf("%d;charindex:%d\n",j,charindex);
 	//	printf("%d;charcount:%d\n",j,charcount);
 		break;
	}
//	printf("%d;charindex:%d\n",j,charindex);
// 	printf("%d;charcount:%d\n",j,charcount);	
	charcount=0;
 }
 *data=charcount;
 return platecharsig;
}
int getdashplace(int charcount,int Symbol,CvRect*charfeature,stack*area3){
	int dashplace=0,dashnum=0,i;
//areasort(charfeature,charcount);
for(i=0;i<charcount;i++){
	dashplace++;
	if(charfeature[i].x>area3[Symbol].x)
		break;	   
}
//printf("dashplace:%d\n",dashplace);
return dashplace;
}


void charsegmenstep(IplImage * img,int n1,int charnum/**/){
	/******************************************************/
////////////////宣告變數///////////////////	
	clock_t start_time, end_time;
	float total_time = 0;
	start_time = clock(); 
	char filename[]="a.bmp"; 
	int topline=0,leftline=0,deltx=0,delty=0,height=0,width=0;
	int i,j,num=1,counts1=0,num1=0;
	double atan1=0;
	int T=0,tempT=0;
	IplImage *colorImg,* grayImg1,*hmatrix; 
	colorImg =cvLoadImage(filename,CV_LOAD_IMAGE_UNCHANGED);
	grayImg1 = cvCreateImage(cvSize(img->width,img->height),8,1); 
	int numa=0;  
    CvRect * ect;
	int tcutnum=0,dcutnum=0,leftcutnum=0,rightcutnum=0,whitenum=0,tcutmax=0,dcutmax=0;   
	stack *temparea;
    int aaa=0,Symbol=0;
	int symbolcount=0,event;
	IplImage *cuttemp,*cuttemp2;
    int indexsymbol=0;
    int *symbolindexarrray=0; 
	double thear=0; 
	IplImage *vvtesttemp;
	CvRect *ect_1; 
	int n=0,platecontour1=0;
	stack *area1,*area2,*area3;
	IplImage *cutImg,*cuttest;
    int platecontour=0;
    int charcount=0,charindex=0,platecharsig=0;
    CvRect* charfeature;
    float w1;
    int dashplace=0,dashnum=0;
    stack* symbolarrray;
    int symbolindex=0;
    int checknum=0,errorcharsig=0;
    stack* xmatrix;
	
    //IplConvKernel* mask1;
/******************************************************/     
	/******************************/ 
	//////////////轉灰階/////////////////// 	
	colortograyimg(img,grayImg1);
	/////////////////////////////////////////////////////////////////
	//////////二值化////////////
	cvThreshold(grayImg1, grayImg1, 0, 255,  CV_THRESH_OTSU);
	  //////////變成背黑前白處理參數//////////
	parameterplateimg( grayImg1); 
	/******************************/  
	/************************/ 
	//////////前置影像處理(灰階作二值化) //////////////// 
    //cvThreshold(grayImg,grayImg, 0, 255,  CV_THRESH_OTSU); 
    //cvShowImage("binaryimg",grayImg1);
    //cvWaitKey(0);
    /************************/ 
	/******************************/  
	//////////裁邊處理演算法(Chang's method) //////////////// 
    xmatrix=(stack *) malloc((img->height*img->width)*sizeof(stack));
  /*******************************/   
	    ///裁邊///
///搜尋cutleft線///    
	event=1;
	leftcutnum=cutparameter( grayImg1, event);
	//leftcutnum=0;
///搜尋cutright線///
    event=2;
	rightcutnum=cutparameter( grayImg1, event); 
	//rightcutnum=0;   
///搜尋cuttop線///
    event=3;
	tcutmax=cutparameter( grayImg1, event);
	//tcutmax=0; 
	///搜尋cutdown線///
	event=4;
	dcutmax=cutparameter( grayImg1, event);
	//dcutmax=0; 
/*	printf("\nleftcutnum:%d",leftcutnum);	
	printf("\nrightcutnum:%d",rightcutnum);
	printf("\ntcutmax:%d",tcutmax);	
	printf("\ndcutmax:%d",dcutmax);*/
	///////裁邊後顯示所需參數///////
	int errorcut=0;
	if((rightcutnum>leftcutnum) && (dcutmax>tcutmax)){
		cutImg = cvCreateImage(cvSize(rightcutnum-leftcutnum,dcutmax-tcutmax),8,1);
	    cuttest=cvCreateImage(cvSize(rightcutnum-leftcutnum,dcutmax-tcutmax),8,1);
	    cutimg(cutImg,cuttest,grayImg1, tcutmax, dcutmax,  leftcutnum, rightcutnum);
	}
	else
	    errorcut=1;
    if(errorcut!=1){
	//////////變成背黑前白處理參數//////////
	   parameterplateimg( cutImg);
///////裁邊後結果(顯示)///////	      
	   //cvShowImage("cutImg",cutImg);///////////////
       //cvWaitKey(0);	
/*******************************/	
/*******************************/ 	
    ///連通法segment(計數次數)///
       numa=conponentnumseg(cuttest,cutImg);
       //printf("\nnuma:%d",numa);
		 	 	 
///連通法(標記位置)///
       ect = (CvRect *) malloc(numa*sizeof(CvRect));
       labelconponent(cutImg,ect );

///連通法(標記位置排序)///	
///////////////排序x點由小到大///////////////
	   area1=(stack *) malloc(numa*sizeof(stack));
	   area2=(stack *) malloc(numa*sizeof(stack));
	   for(i=0;i<numa;i++){
		   area1[i].label=0;
		   area1[i].state=0;
		   area1[i].index=0;
		   area1[i].x=0;
		   area1[i].y=0;
		   area1[i].height=0;
		   area1[i].width=0;
		   area2[i].label=0;
		   area2[i].state=0;
		   area2[i].index=0;
		   area2[i].x=0;
		   area2[i].y=0;
		   area2[i].height=0;
		   area2[i].width=0;
	   }	
	   areasort(ect,numa);	
	
///////////////連通法(標記位置排序)結果///////////////
       //printf("\n");
       //printf("\n");
/************************************************/
/************************************************/
////搜尋字元和分隔號特徵及判斷////
//int checknum=0,errorcharsig=0;
       neighborchar( numa, area1, area2,ect,cutImg);
       
       /* for(i=0;i<numa;i++)
	          if(area1[i].state==1)
                 cvRectangle( cutImg, cvPoint( area1[i].x, area1[i].y ),
		         cvPoint( area1[i].x + area1[i].width, area1[i].y +area1[i].height ), 
		         cvScalar(255,255,0,0), 2,2,0 );/**/
		 
          /*cvShowImage("labelcutImg",cutImg);/////////////
          cvWaitKey(0);	 */ 
       
       for(i=0;i<numa;i++)
	      if(area2[i].state==1)
	         checknum++;
       //printf("checknum:%d",checknum);
       if(checknum<6 || checknum>9)
          errorcharsig=1;
       if(errorcharsig!=1 ){
/************************************************/
/************************************************************/
//////////找車牌字元+車牌輪廓+破折號//////////
          temparea = (stack*) malloc(numa*sizeof(stack));
/////找車牌字元(+車牌輪廓)區塊/////
/////找車牌字元(+車牌輪廓)區塊標記/////
          /*for(i=0;i<numa;i++)
	          if(area2[i].state==1)
                 cvRectangle( cutImg, cvPoint( area1[i].x, area1[i].y ),
		         cvPoint( area1[i].x + area1[i].width, area1[i].y +area1[i].height ), 
		         cvScalar(255,255,0,0), 2,2,0 );/**/
		 
          /*cvShowImage("labelcutImg",cutImg);/////////////
          cvWaitKey(0);	 */ 
/////找車牌字元(+車牌輪廓)區塊備份/////
          for(i=0;i<numa;i++){
	          if(area1[i].state==1){
	             aaa++;
	             temparea[aaa-1].x=area1[i].x;
	             temparea[aaa-1].y=area1[i].y;
	             temparea[aaa-1].width=area1[i].width;
	             temparea[aaa-1].height =area1[i].height;	
	          }		
          }


////////找車牌字元(+車牌輪廓)區塊氣泡排序////////
          areasort1(temparea,aaa);
/************************************************************/ 
////////////搜尋字元演算法////////////
          counts1=readcharfeature(temparea,aaa,sT,cuttest,platecontour,platecontour1);

///////////////讀取字元////////////////// 
 	      cuttemp2 = cvCreateImage(cvSize(cuttest->width,cuttest->height),8,1);
	      for(j=0;j<cuttest->height;j++)
	         for(i=0;i<cuttest->width;i++){
	   	         cuttemp2->imageData[(j) *  cutImg->widthStep + i ]=0;
	         }	 
          for(i=0;i<counts1;i++)
	   	      cuttemp2->imageData[(sT[i].y) *  cuttemp2->widthStep + sT[i].x ]=255;  	   	
	      //printf("\n\n%d;event:%d\n",i,dx);		 
          //printf("%d;counts1:%d\n",i,counts1);  
/************************************************************/ 
/************************************************************/  
////********水平校正演算法********************////   
	/*******K-mean演算法 ***************/ 
		  //****類別平均初始化****//		  
/////////////////////////////	         		       	
    ////k-mean演算法(code)結束//// 
    ////求得傾斜角度//// 	
	       atan1=kmeanslop(counts1,cuttemp2,sT,s1,s2,1);
	       thear=-atan1;//校正角度 
	////求得傾斜角度(結束)//// 
	////顯示相關參數資訊//// 
	     //printf("\n thear:%lf\n",thear);     
	       //printf("\n atan:%lf\n",atan1);
	////顯示相關參數資訊(結束)//// 
/*******水平校正演算法(對象:字元*7+邊緣車牌) ***************/ 
           correct( xmatrix,sT,counts1,thear,1,0,0);
/*******水平校正演算法(結束) ***************/	
/*******水平校正之顯示(cvShowImage)所需參數 ***************/
           correctinf=hcorrectinf(xmatrix,counts1 ,4);
	       deltx=correctinf.x;
	       delty=correctinf.y;	
	       width=correctinf.width;
	       height=correctinf.height;
	////////////校正後適當空間宣告//////
	       hmatrix= cvCreateImage(cvSize(width,height),8,1);
	       for(j=0;j<hmatrix->height;j++)
	          for(i=0;i<hmatrix->width;i++)
		          hmatrix->imageData[j*hmatrix->widthStep+i]=0;/**/ 	   	     
	       for(i=0;i<counts1;i++){
		       hmatrix->imageData[(xmatrix[i].y+delty)*hmatrix->widthStep+(xmatrix[i].x+deltx)]=255;
	       }
/*******水平校正之顯示(cvShowImage)所需參數(結束) ***************/			           
/************顯示 ************/ 
	      //cvShowImage("cutImg",cutImg); //裁邊後 		
	       //cvShowImage("correctimg",hmatrix);//校正後 
	      //cvShowImage("readcharimg",cuttemp2); //讀取字元 
           /*cvWaitKey(0);*/
/************水平校正結束 ************/       
/*********分割(字元)處理演算法(連通法) ***************/ 
	    ///連通法///
	/////取得連通法標記數量/////  
	       vvtesttemp = cvCreateImage(cvSize(hmatrix->width,hmatrix->height),8,1);
	       backupimg(hmatrix,vvtesttemp);
           num=conponentnumseg(vvtesttemp ,hmatrix);
          // printf("\n num:%d\n",num);
	/////取得連通法標記數量(結束)/////  
/*************************/
/////取得連通法標記座標///// 
           //int num1;
           ect_1 = (CvRect *) malloc(num*sizeof(CvRect));
           num1=labelconponent(vvtesttemp,ect_1 );
          // printf("\n num1:%d\n",num1);
/////取得連通法標記座標(結束)///// 	
//********************//
/////取得連通法標記矩形/////	
           /*for( i = 0;i < num1;i ++){
		     cvRectangle( vvtest, cvPoint( ect_1[i].x, ect_1[i].y ),
		     cvPoint( ect_1[i].x + ect_1[i].width, ect_1[i].y + ect_1[i].height ), 
		     cvScalar(255,255,0,0), 2,2,0 );
	       }*/ 
/////取得連通法標記矩形(結束)/////
///排序
           areasort(ect_1,num1);
////////////////////////////////////////////////////////////////
           area3 = (stack *) malloc(num1*sizeof(stack));

           for(i=0;i<num1;i++){
	           area3[i].x=ect_1[i].x;
	           area3[i].y=ect_1[i].y;
	           area3[i].width=ect_1[i].width;
	           area3[i].height=ect_1[i].height;
	           area3[i].state=1;
	
           }
           symbolarrray= (stack*) malloc(num1*sizeof(stack));	
           for(j=0;j<num1;j++){
		       symbolcount++;
		       symbolarrray[symbolcount-1].x=area3[j].x;
		       symbolarrray[symbolcount-1].y=area3[j].y;
		       symbolarrray[symbolcount-1].width=area3[j].width;
		       symbolarrray[symbolcount-1].height=area3[j].height;
           }/*	*/
////////////////找破折號////////////////
           //Symbol=finddashchar(numa,area1,cutImg);//校正前取得破折號位置 
           Symbol=finddashchar(num1,area3,cutImg);//校正後取得破折號位置 
///////////////////計算破折號位置/////////////////// 
           /*symbolindexarrray= (int*) malloc(symbolcount*sizeof(int));
           //symbolindex=platechararray(symbolindexarrray,numa,area1,Symbol);
           symbolindex=platechararray(symbolindexarrray,num1,area3,Symbol);	   
           printf("\n symbolindex:%d\n",symbolindex+1); */
///////////////////////////////////////////////////		
		   //printf("\n Symbol:%d\n",Symbol);
  ///////////////////標記破折號位置//////////////////
         /*  cvRectangle( hmatrix, cvPoint( area3[Symbol].x, area3[Symbol].y ),
		   cvPoint( area3[Symbol].x + area3[Symbol].width, area3[Symbol].y + area3[Symbol].height ), 
		   cvScalar(255,255,0,0), 2,2,0 );*/ 
		  //cvShowImage("dashlabel",hmatrix);/* //	

/************************************************************/
/////////////////////////////////////////
///////車牌輪廓濾除(區域相似度)/////////

           charfeature = (CvRect *) malloc(symbolcount*sizeof(CvRect));
           platecharsig=getplatecharvector(symbolcount,symbolarrray,charfeature,cutImg,&charcount);
	
//////////////取得破折號位置//////////////////
           //areasort(charfeature,charcount);
          
		   dashplace=getdashplace(charcount,Symbol,charfeature,area3);

///////////////////////////////
           /*for(i=0;i<charcount;i++)
               cvRectangle( hmatrix, cvPoint( charfeature[i].x, charfeature[i].y ),
		       cvPoint( charfeature[i].x + charfeature[i].width, charfeature[i].y + charfeature[i].height ), 
		       cvScalar(255,255,0,0), 2,2,0 );*/ 
		    //cvShowImage("charfeature",hmatrix); // 
///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////	
///裁邊處理/// 
            //n=cutsmallchar(num1,ect_1,hmatrix);
            n=0;
            //printf("platecharsig:%d\n",platecharsig);
            if(platecharsig==1 && (dashplace>2 && dashplace<6 )){
            	printf("%d_%d.bmp:\n",charnum,n1);
            	n=cutcharfeature(charcount,charfeature,hmatrix,charnum,n1,dashplace);
            	counter++;
            
			}/**/
              
            if(n>5 && n<=7){
            	
            	//printf("車牌資訊\n");
                 printf("dashplace:%d\n",dashplace);
				 printf("字元數n:%d\n",n); 
				 
			}
            //cvShowImage("finalimg",hmatrix);///////////
            //cvWaitKey(0);  
           /* else
               printf("不是車牌資訊");*/ 
            /*	 n=0; */ 
/*************************/
/***********顯示**************/
            cvShowImage("finalimg",hmatrix);///////////
            cvWaitKey(0);
/************結束 ************/ 
       }
    }
	
}

//分割影像//
void cutsmallimg(int num,CvRect * ect,IplImage *colorImg,IplImage *grayImg,int charnum/**/){
	int ii,n=0,i,j,min;
	CvSize  size1;
	double w1,den;
	int count,count2;
	IplImage *pDstImg = NULL; 
	CvMat *mat_roi;
	char chr[10];
	char char1[20];
	char str[10]=".bmp";
	
	 for(ii=0;ii<num;ii++){
   CvSize size1 = cvSize(ect[ii].width, ect[ii].height);
   w1=(float)(ect[ii].width+1)/(ect[ii].height+1);////車牌比例 
   if(w1>1.5 && w1<4.6){
	   count=0;
       count2=0;
       pDstImg = cvCreateImage( size1, IPL_DEPTH_8U, 3);
       mat_roi = cvCreateMat(ect[ii].height, ect[ii].width,CV_8UC3);
       cvGetSubRect(grayImg, mat_roi, ect[ii]);
       cvGetImage(mat_roi, pDstImg);
       cvThreshold(pDstImg, pDstImg, 0, 255,  CV_THRESH_OTSU); 
       for( i = 0;i < pDstImg->height;i ++)
          for( j = 0;j < pDstImg->width;j ++){
  	         if(((unsigned char *)pDstImg->imageData)[i * pDstImg->widthStep + j]==0)
  	           count=count+1;
  	         if(((unsigned char *)pDstImg->imageData)[i * pDstImg->widthStep + j]==255)
  	           count2=count2+1;
          }
        min=count;
        if(count2<min)
           min=count2;
	   den=(float)min/(pDstImg->height*pDstImg->width);
       if(den>0.14 && den<0.5){
       	    if(ect[ii].width<400)//
       	      if(ect[ii].height<200)
    	  if(pDstImg->height>0.024*grayImg->height)/**/{
		     cvRectangle( colorImg, cvPoint( ect[ii].x, ect[ii].y ),cvPoint( ect[ii].x + ect[ii].width, ect[ii].y + ect[ii].height ), cvScalar(0,0,255,0), 2,2,0 );
             cvGetSubRect(colorImg, mat_roi, ect[ii]);
             cvGetImage(mat_roi, pDstImg);	
             n++;
           sprintf(chr, "%d", n);
           sprintf(char1, "%d", charnum);
           strcat(char1,"_" );
           strcat(char1,chr );
           strcat(char1, str); 
		   charsegmenstep(pDstImg,n,charnum/**/);
           cvSaveImage(char1,  pDstImg, 0 );	
           cvReleaseMat(&mat_roi);//釋放資源
           //cvReleaseImage(&pDstImg);//釋放資源 
           }
	    }  
     }
   }
}


/************************************************************************/

int main(int argc, char *argv[])
{
	
	clock_t start_time, end_time;
	float total_time = 0;
	start_time = clock(); 
	int num=0,num1=0;
    IplImage *pDstImg = NULL;
    char filename[]="196.jpg"; 
    //char filename[]=argv[1];79;71;101;143;104;189.jpg;
    int i,j,k;
    float r=0.12;
    float q=0.02;
    int T=0;
    double w=0; 
    IplImage *colorImg; 
    int * test,count3=0;  
    CvRect *ect; 
    IplConvKernel* mask1;
    int endsignal[4],endcount=0;
    //********colorimage************************************************
    //colorImg = cvLoadImage(argv[1],CV_LOAD_IMAGE_UNCHANGED); // 讀取影像的資料結構
    colorImg = cvLoadImage(filename,CV_LOAD_IMAGE_UNCHANGED);
    IplImage* testimg = cvCreateImage(cvSize(colorImg->width,colorImg->height),8,1);
    //IplImage* gradiimg = cvCreateImage(cvSize(colorImg->width,colorImg->height),8,1);
    IplImage* grayImg = cvCreateImage(cvSize(colorImg->width,colorImg->height),8,1); 
    test = (int *) malloc(colorImg->height * colorImg->widthStep*sizeof(int));
    if(!colorImg){
        printf("Error: Couldn't open the image file.\n");
    }
  //**********gray****************************************************************
    colortograyimg(colorImg,grayImg);
    cvSaveImage("colorImg.jpg",  colorImg, 0 );
    cvSaveImage("grayImg.jpg",grayImg, 0 );
    //cvEqualizeHist(grayImg,grayImg);//影像平均化
    cvSmooth(grayImg,grayImg,CV_BLUR,3,3,0,0);
    backupimg(grayImg,testimg); 
    
  //****************************************************************************************
  //***************gradiimage******************
    gradiimg(testimg,test);
    
   // absgradimg(testimg,test);
    //cvSaveImage("testimg.jpg",testimg, 0 );
    //cvShowImage("HelloWorld-2",testimg);
    //cvWaitKey(0);
  //*****************end*****************
  //********threshod*********************************************************
    do{
      T=suitthreshod(testimg,test,r);
      absgradimg(testimg,test);
      cvSaveImage("testimg.jpg",testimg, 0 );
      //cvShowImage("HelloWorld-2",testimg);
      //cvWaitKey(0);
  /////***************************
      graytobinary(T,testimg);
      //cvSaveImage("binaryimg.jpg",testimg, 0 );
     //cvShowImage("binaryimg",testimg);//顯示梯度二值化結果 
     // cvWaitKey(0);/**/
  //////***************************
/*****
形態學 
*****/
      mask1=cvCreateStructuringElementEx( 9, 9,4, 4, CV_SHAPE_RECT,0 );
      cvDilate(testimg,testimg,mask1,1);
      cvErode(testimg,testimg,mask1,1);
      mask1=cvCreateStructuringElementEx(3, 3,1, 1, CV_SHAPE_RECT,0 );
      cvErode(testimg,testimg,mask1,1);
      cvDilate(testimg,testimg,mask1,1);
      mask1=cvCreateStructuringElementEx( 3, 3, 1, 1, CV_SHAPE_RECT,0 );
      cvErode(testimg,testimg,mask1,1);
      mask1=cvCreateStructuringElementEx( 7,7, 3,3, CV_SHAPE_RECT,0 );
      cvDilate(testimg,testimg,mask1,1);  
/**/
  //****density***//
      w=wholedensity(testimg);
      //printf("w:%d\n",w);
      if(w>0.1)
        r=r+q;
      if(w<0.021)
  	    r=r-q;
  	    
  	   // end_time = clock();
       // total_time = (float)(end_time - start_time)/CLOCKS_PER_SEC;
      //  if(total_time>10)
        //  break;
  	    
   }while(w>0.126 || w<0.021);/**********經形態學後，在整張圖片分布比w>0.26 || w<0.021*/ 
//**********************************************************************

   cvSaveImage("testimg_1.jpg",testimg, 0 );
   cvShowImage("HelloWorld-2",testimg);//顯示梯度影像可調變門檻結果 
   cvWaitKey(0);/**/
//***plate search*********************************************************	 
    num=conponentnum(testimg,test);
   //************************************************************************
    ect = (CvRect *) malloc(num*sizeof(CvRect));
    num=labelconponent(testimg,ect ) ;
	keyr = (stack *) malloc(num*sizeof(stack));//記錄合併頭 
	keyfr= (stack *) malloc(num*sizeof(stack));//重疊(兩經合併處理區塊)處理後存放 
	keynew= (stack *) malloc(num*sizeof(stack));// 合併最終結果(重疊+合併處理+刪減合併內小區塊) 
	for(i=0;i<num;i++){
		keyr[i].x=0;
		keyr[i].y=0;
		keyr[i].width=0;
		keyr[i].height=0;
		keyr[i].state=0;
		keyr[i].index=0;	
		keyfr[i].x=0;
		keyfr[i].y=0;
		keyfr[i].width=0;
		keyfr[i].height=0;
		keyfr[i].state=0;
		keyfr[i].index=0;		
	}	
  //----------------排序x點由小到大--------------
  	areasort(ect,num);   
  //--------------------排序後結果-------------------------
    /* printf("\n");
     printf("\n");
     for(i=0;i<num;i++){
    	printf("ect[%d].x=%d;",i+1,ect[i].x);
    	printf("ect[%d].y=%d;",i+1,ect[i].y);
    	printf("ect[%d].width=%d;",i+1,ect[i].width);
    	printf("ect[%d].height=%d;",i+1,ect[i].height); 
		printf("p1_hy=%d;",ect[i].y+ect[i].height);
		printf("p1_hx=%d\n",ect[i].x+ect[i].width);          
	 }
    */ 
  //----------------排序x點由大到小--------------	
    /*-----------------合併判斷-----------------------*/
    k=0;
    while(1){//重新合併次數 
            count3=neighbormerger(k,num,count3,ect,keyr);
            neighbordatarenew(ect,keyr,num);
	        mergeoverlap(count3,keyr,keyfr);
	        overlapdatarenew(ect,keyr,keyfr,num);
	        if(k%2!=0)
	           endsignal[k%4]=count3;
		    else if(k%2==0){
		       endsignal[k%4]=count3;	
		    }
	   	    if(endsignal[0]==endsignal[2] && endsignal[1]==endsignal[3] || k==2 )
	   	       break;
	        printf("count3[%d]:%d\n",k,count3);
            count3=0;      
	        for(i=0;i<num;i++){
		        keyr[i].x=ect[i].x;
		        keyr[i].y=ect[i].y;
		        keyr[i].width=ect[i].width;
		        keyr[i].height=ect[i].height;
		        keyr[i].index=0;
		        keyr[i].indexj=0;
		        keyr[i].indexi=0;	
		        keyr[i].state=0;
		        keyfr[i].state=0;
		        keyfr[i].statej=0;
		        keyfr[i].statei=0;
		        keyfr[i].x=ect[i].x;
                keyfr[i].y=ect[i].y;
                keyfr[i].width=ect[i].width;
                keyfr[i].height=ect[i].height;
	        } 
	k++;
    }	
   /*-----------------重新 plate search-----------------------*/
   char* charnum;
   int intnum;
   charnum=strtok(filename, ".jpg");//字串分割 
   intnum=atoi ( charnum ); // 將字串轉整數
   printf("測試:%d.jpg[\n",intnum);
   //printf("intnum:%d",intnum);
   cutsmallimg(num,ect,colorImg,grayImg,intnum);
    //cvShowImage("labelcutimg",colorImg);//符合車牌特徵標記出來 
    //cvSaveImage("110.bmp",  colorImg, 0 );
   /*--------------------結束---------------------*/
   printf("車牌擷取總數:%d]\n",counter);
 //*************************************************************************** /  
   // for(i=0;i<num;i++)
      // cvRectangle( colorImg, cvPoint( ect[i].x, ect[i].y),
	 //  cvPoint( ect[i].x + ect[i].width, ect[i].y + ect[i].height ), cvScalar(255,255,0,0), 2,2,0 );                    	                                  
    cvShowImage("labelarea",colorImg);//候選區塊/* */
    cvSaveImage("labelarea.jpg",colorImg, 0 );
  end_time = clock();
  total_time = (float)(end_time - start_time)/CLOCKS_PER_SEC;
  printf("\n經過了 %f 秒....\n",total_time);
  cvReleaseImage(&grayImg);
  cvReleaseImage(&testimg);
  cvReleaseImage(&colorImg);	
  free(ect);
  free(keyr);
  free(keyfr);
  free(keynew);
  free(test);
  cvWaitKey(0);
  system("pause");
  return 0;
}
