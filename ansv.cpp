//*****************THINGS TO FIX**************************
//********************************************************
//also the bug of repeated output of insertion or deletion
//also check findDupRef output. use 2L 6100162  as the test case
//Get rid of coverage storage [future] and use overlapping but non-embedded MUMs as criteria for removal of both MUMs from getting into the preliminary cm list.
//split the overlapInRef and overlapInQ
//********************************************************
//********************************************************

#include "sv.h"
#include<iostream>

using namespace std;
using chroms = map<string,chromPair>;
using ccov = vector<int>;
using vq = vector<qord>;

void annotGaps(vector<mI> & cm, ccov & masterRef, ccov & masterQ, ccov & chromDensityRef, ccov & chromDensityQ,vector<mI> & cnv, map<int,vector<qord> > & umRef, string & refseq, string & qseq, vector<int> & seqLen,ofstream & fout, ofstream & fsmall, int & id)
{
	vector<double> cov(2);//stores coverage for ref and query intervals
	vector<int> vi;
	mI gapmi,cnrmi,cnqmi,thismi,nextmi,tempmi;
	cnqmi.x1 = 0;//initialize
	cnrmi.x1 = 0;//initialize to avoid surprises :)
	vector<mI> storedCNV, tempVmi,indels;
	double cnvOvl =0; //checks whether ref and query CNVs overlap
	tempVmi = cm;
	sort(tempVmi.begin(),tempVmi.end(),qusort);//sort it by Q cords for resolving for-rev junctions
	for(unsigned int i=0; i<cm.size()-1;i++)
	{
//cout<<"prev-this\t"<<thismi.rn<<'\t'<<thismi.x1<<'\t'<<thismi.x2<<'\t'<<thismi.qn<<'\t'<<thismi.y1<<'\t'<<thismi.y2<<endl;		
//cout<<"this\t"<<cm[i].rn<<'\t'<<cm[i].x1<<'\t'<<cm[i].x2<<'\t'<<cm[i].qn<<'\t'<<cm[i].y1<<'\t'<<cm[i].y2<<endl;
		if((cm[i].y1 >cm[i].y2) && (thismi.y1 <thismi.y2))//if last was forward and this is inverted
		{
			fout<<thismi.rn<<"\t"<<thismi.x2<<"\t"<<thismi.x2<<"\tINV\t"<<thismi.qn<<"\t"<<thismi.y2<<"\t"<<cm[i].y1<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<cm[i].y1-thismi.y2<<"\t"<<nearestInt(cov[0])<<"\t"<<nearestInt(cov[1])<<endl;
		}
		if((cm[i].y1 <cm[i].y2) && (thismi.y1 >thismi.y2))//if this is forward and previous was inverted
		{
			fout<<thismi.rn<<"\t"<<thismi.x2<<"\t"<<thismi.x2<<"\tINV\t"<<thismi.qn<<"\t"<<thismi.y2<<"\t"<<cm[i].y1<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<cm[i].y1-thismi.y2<<"\t"<<nearestInt(cov[0])<<"\t"<<nearestInt(cov[1])<<endl;
		}

		thismi = cm[i];
		nextmi = cm[i+1];
		gapmi.rn = thismi.rn;//should be same chromosomes
		gapmi.qn = thismi.qn;//same as above
		
		//tempmi = returnMumByQ1(thismi.y1,tempVmi);//returns the next mi.y1
//do some check on the query coordinate for translocations
		if(thismi.y1 <thismi.y2) // if this is forward strand
		{
			tempmi = returnMumByQ1(thismi.y1,tempVmi);//returns the next mi.y1
			if(!(tempmi == nextmi))//tempmi isn't the next one
			{
				nextmi.y1 = tempmi.y1;//transfer the query coordinates
				nextmi.y2 = tempmi.y2;
				nextmi.c = 't';//to indicate transposed or inverted
			}
		}
		if(thismi.y1 > thismi.y2)//if this is reverse strand
		{
			tempmi = returnMumByQ2(thismi.y1,tempVmi);//returns the previous mi.y1
			if(!(tempmi == nextmi))//next is not part of the same inversion
			{
				tempmi = returnMumByQ1(thismi.y1,tempVmi);//returns the next mi.y1
				nextmi.y1 = tempmi.y1;
				nextmi.y2 = tempmi.y2;
			}
		}
		//callSmall(tempmi,umRef,refseq,qseq,seqLen,fsmall);//activate this to call SNPs and small indels
		if((thismi.y1 > thismi.y2) && (nextmi.y1 > nextmi.y2)) // two subsequent mums are inverted
		{
			gapmi.x1 = thismi.x2;
			gapmi.x2 = max(nextmi.x1,thismi.x2);
			//gapmi.y1 = max(nextmi.y1,thismi.y2);//maintain the forward strand style for gaps
			gapmi.y2 = thismi.y2;
			 //gapmi.y2 = thismi.y2;
			//gapmi.y1 = thismi.y1;
			gapmi.y1 = min(nextmi.y1,thismi.y2);//maintain the forward strand style for gaps
			gapmi.c = 'i';//not needed but bookeeping
			cnqmi = findDupQ(thismi,nextmi);
                 //       fout<<thismi.rn<<"\t"<<thismi.x1<<"\t"<<thismi.x2<<"\tINV\t"<<thismi.qn<<"\t"<<thismi.y2<<"\t"<<thismi.y1<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<cm[i].x2-cm[i].x1<<"\t"<<nearestInt(cov[0])<<"\t"<<nearestInt(cov[1])<<endl;
		}
		if((thismi.y2 > thismi.y1) && (nextmi.y2 > nextmi.y1)) // both are forward
		{
			gapmi.x1 = thismi.x2;
			gapmi.x2 = max(nextmi.x1,thismi.x2);
			gapmi.y1 = thismi.y2;
			gapmi.y2 = max(nextmi.y1,thismi.y2);
			cnqmi = findDupQ(thismi,nextmi);
		}
		if((thismi.y2 > thismi.y1) && (nextmi.y1 > nextmi.y2)) //first is forward but second is reverse
		{
			gapmi.x1 = thismi.x2;
			gapmi.x2 = max(nextmi.x1,thismi.x2);
			gapmi.y1 = thismi.y2;
			gapmi.y2 = max(min(tempmi.y1,tempmi.y2),thismi.y2);
			cnqmi = findDupQ(thismi,tempmi);
			cov = getCoverage(gapmi,masterRef,masterQ);
//			fout<<thismi.rn<<"\t"<<gapmi.x1<<"\t"<<gapmi.x2<<"\tINV\t"<<thismi.qn<<"\t"<<gapmi.y1<<"\t"<<gapmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<max(gapmi.x2-gapmi.x1,gapmi.y2-gapmi.y1)<<"\t"<<nearestInt(cov[0])<<"\t"<<nearestInt(cov[1])<<endl;
		}
		if((i>0) && (thismi.y1 > thismi.y2) && (nextmi.y2 > nextmi.y1)) //reverse followed by forward
		{
			gapmi.x1 = thismi.x2;
			gapmi.x2 = max(nextmi.x1,thismi.x2);
			gapmi.y1 = thismi.y1;
			gapmi.y2 = max(max(thismi.y1,thismi.y2),nextmi.y1);
			cnqmi = findDupQ(thismi,nextmi);
//			fout<<thismi.rn<<"\t"<<gapmi.x1<<"\t"<<gapmi.x2<<"\tINV\t"<<gapmi.qn<<"\t"<<gapmi.y1<<"\t"<<gapmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<max(gapmi.x2-gapmi.x1,gapmi.y2-gapmi.y1)<<"\t"<<nearestInt(cov[0])<<"\t"<<nearestInt(cov[1])<<endl;
		}
//cout<<"this\t"<<thismi.rn<<'\t'<<thismi.x1<<'\t'<<thismi.x2<<'\t'<<thismi.qn<<'\t'<<thismi.y1<<'\t'<<thismi.y2<<endl;		
//cout<<"temp\t"<<tempmi.rn<<'\t'<<tempmi.x1<<'\t'<<tempmi.x2<<'\t'<<tempmi.qn<<'\t'<<tempmi.y1<<'\t'<<tempmi.y2<<endl;
//cout<<"next\t"<<nextmi.rn<<'\t'<<nextmi.x1<<'\t'<<nextmi.x2<<'\t'<<nextmi.qn<<'\t'<<nextmi.y1<<'\t'<<nextmi.y2<<endl;
		cnrmi = findDupRef(thismi,nextmi);//find duplicates of ref segments in query
		cnvOvl = double(min((cnrmi.x2-cnrmi.x1),(cnqmi.x2-cnqmi.x1)))/double(max((cnrmi.x2-cnrmi.x1),(cnqmi.x2-cnqmi.x1)));
//cout<<"CNR\t"<<cnqmi.rn<<'\t'<<cnqmi.x1<<'\t'<<cnqmi.x2<<'\t'<<cnqmi.qn<<'\t'<<cnqmi.y1<<'\t'<<cnqmi.y2<<endl;
//cout<<"CNQ\t"<<cnrmi.rn<<'\t'<<cnrmi.x1<<'\t'<<cnrmi.x2<<'\t'<<cnrmi.qn<<'\t'<<cnrmi.y1<<'\t'<<cnrmi.y2<<endl;
		if((cnrmi.x1 != 0) && (cnvOvl <0.3))//you can't have CNV both in reference and query
		{
			cov =getCoverage(cnrmi,masterRef,masterQ,0.3); 
			if((nearestInt(cov[0]) >2) || nearestInt((cov[1]) >2)) 
			{
				fout<<cnrmi.rn<<"\t"<<cnrmi.x1<<"\t"<<cnrmi.x2<<"\tnCNV-Q\t"<<cnrmi.qn<<"\t"<<cnrmi.y1<<"\t"<<cnrmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<(cnrmi.x2 -cnrmi.x1)<<"\t"<<cov[0]<<"\t"<<cov[1]<<endl;
			}
			else
			{
				fout<<cnrmi.rn<<"\t"<<cnrmi.x1<<"\t"<<cnrmi.x2<<"\tCNV-Q\t"<<cnrmi.qn<<"\t"<<cnrmi.y1<<"\t"<<cnrmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<(cnrmi.x2 -cnrmi.x1)<<"\t"<<cov[0]<<"\t"<<cov[1]<<endl;
			}
		}
		if((cnqmi.x1 != 0) && (cnqmi.x2 - cnqmi.x1 != 0))
                {
			cov =getCoverage(cnqmi,masterRef,masterQ,0.3);
			if((nearestInt(cov[0]) >2) || (nearestInt(cov[1]) >2))
			{
				fout<<cnqmi.rn<<"\t"<<cnqmi.x1<<"\t"<<cnqmi.x2<<"\tnCNV-R\t"<<cnqmi.qn<<"\t"<<cnqmi.y1<<"\t"<<cnqmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<(cnqmi.x2 -cnqmi.x1)<<"\t"<<cov[0]<<"\t"<<cov[1]<<endl;
			}
			else
			{
				fout<<cnqmi.rn<<"\t"<<cnqmi.x1<<"\t"<<cnqmi.x2<<"\tCNV-R\t"<<cnqmi.qn<<"\t"<<cnqmi.y1<<"\t"<<cnqmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<(cnqmi.x2 -cnqmi.x1)<<"\t"<<cov[0]<<"\t"<<cov[1]<<endl;
			}
		}
		cnrmi.x1 = 0;//reset cnvmi.x1
		cnqmi.x1 = 0;//reset cnqmi.x1;
		cnvOvl = 0;
		findCnvOverlap(gapmi,cnv,storedCNV, indels,masterRef, masterQ,chromDensityRef,chromDensityQ,fout,id);
		storedCNV.clear();//flush out the CNV vector
		indels.clear();//flush out the indel vector
	}
} 
//////////////////////////////////////////////////////////////////////////////////	
void findCnvOverlap(mI & gapmi,vector<mI> ncm, vector<mI> & cnv, vector<mI> & indels,ccov & masterRef, ccov & masterQ,ccov & chromDensityRef,ccov & chromDensityQ,ofstream & fout, int & id) //returns a cnv if it overlaps a gap
{
	mI tempmi;//tempmi is cnv and mi is the gapmi
	mI gapRight,gapLeft;//two sides of the new split gap
	vector<mI> smum; //selected mums that overlap with the gap.
	double refProp = 0 , qProp = 0;//stores total overlap proportion
	int refOvl =0,qOvl =0;
	vector<double> vd(2),vn(2);

cout<<"gap\t"<<gapmi.rn<<'\t'<<gapmi.x1<<'\t'<<gapmi.x2<<'\t'<<gapmi.qn<<'\t'<<gapmi.y1<<'\t'<<gapmi.y2<<endl;
	if((gapmi.x2 > gapmi.x1) || (gapmi.y2 > gapmi.y1)) //gaps in either ref or query exist, and in forward strand. inverted coordinates are converted to forward coordinates. so gaps are always in forward direction
	{
		for(unsigned int i = 0;i<ncm.size();i++)
		{	
			vn = getChromCount(ncm[i],chromDensityRef,chromDensityQ);
			if((vn[0] < 0.5) && (vn[1] < 0.5) && (ncm[i].x1 != 0) && (ncm[i].y1 != 0))
			{
				if(!(ncm[i].x2 < gapmi.x1) && !(ncm[i].x1 > gapmi.x2) && (ncm[i].rn == gapmi.rn) && (ncm[i].qn == gapmi.qn))
				{
	//				vn = getChromCount(ncm[i],chromDensityRef,chromDensityQ);	
					refOvl = min(ncm[i].x2,gapmi.x2) - max(ncm[i].x1,gapmi.x1);//overlap between gap & ncm
					refProp = double(refOvl)/double(ncm[i].x2-ncm[i].x1);			
					if(refProp >0.1) //at least 50% of the candidate mum overlaps the gap
					{
//cout<<"R-ovl\t"<<gapmi.rn<<'\t'<<gapmi.x1<<'\t'<<gapmi.x2<<'\t'<<gapmi.qn<<'\t'<<gapmi.y1<<'\t'<<gapmi.y2<<'\t'<<ncm[i].x1<<'\t'<<ncm[i].x2<<'\t'<<ncm[i].y1<<'\t'<<ncm[i].y2<<'\t'<<refOvl<<'\t'<<refProp<<endl;
						if(ncm[i].y1 > ncm[i].y2)//if inverted
						{
							ncm[i].y1 = ncm[i].y1 - (max(ncm[i].x1,gapmi.x1)-ncm[i].x1);
							ncm[i].y2 = ncm[i].y2 + (ncm[i].x2 - min(ncm[i].x2,gapmi.x2));
						}
						else
						{
							ncm[i].y1 = ncm[i].y1 + (max(ncm[i].x1,gapmi.x1)-ncm[i].x1);
							ncm[i].y2 = ncm[i].y2 - (ncm[i].x2 - min(ncm[i].x2,gapmi.x2));
						}
						ncm[i].x1 = max(ncm[i].x1,gapmi.x1);//shorten ncm[i] interval to fit in the gap
						ncm[i].x2 = min(ncm[i].x2,gapmi.x2);//shorten as above
						ncm[i].c = 'r';//overlaps reference gaps
						ncm[i].l = refOvl;
						smum.push_back(ncm[i]);
						ncm[i].x1 = 0;//not use it again for gap filling in any subsequent gaps
						ncm[i].x2 = 0;
						ncm[i].y1 = 0;
						ncm[i].y2 = 0;
					}
				}
				if((!(max(ncm[i].y2,ncm[i].y1) < gapmi.y1)) && (!(min(ncm[i].y1,ncm[i].y2) > gapmi.y2))&& (ncm[i].rn == gapmi.rn) && (ncm[i].qn == gapmi.qn))
				{
	//				vn = getChromCount(ncm[i],chromDensityRef,chromDensityQ);
					qOvl = min(max(ncm[i].y1,ncm[i].y2),gapmi.y2) - max(min(ncm[i].y1,ncm[i].y2),gapmi.y1);
					qProp = double(qOvl)/double(abs(ncm[i].y2-ncm[i].y1));
					if((qProp > 0.1) && (find(smum.begin(),smum.end(),ncm[i]) == smum.end()))
					{
cout<<"Q-ovl\t"<<gapmi.rn<<'\t'<<gapmi.x1<<'\t'<<gapmi.x2<<'\t'<<gapmi.qn<<'\t'<<gapmi.y1<<'\t'<<gapmi.y2<<'\t'<<ncm[i].x1<<'\t'<<ncm[i].x2<<'\t'<<ncm[i].y1<<'\t'<<ncm[i].y2<<'\t'<<qOvl<<'\t'<<qProp<<endl;
						if(ncm[i].y1 < ncm[i].y2)
						{
							ncm[i].x1 = ncm[i].x1 + (max(min(ncm[i].y1,ncm[i].y2),gapmi.y1) - ncm[i].y1);
							ncm[i].x2 = ncm[i].x2 - (ncm[i].y2 - min(max(ncm[i].y1,ncm[i].y2),gapmi.y2));
							ncm[i].y1 = ncm[i].y1 + (max(min(ncm[i].y1,ncm[i].y2),gapmi.y1) - ncm[i].y1);
							ncm[i].y2 = ncm[i].y2 - (ncm[i].y2 - min(max(ncm[i].y1,ncm[i].y2),gapmi.y2));
						}
						if(ncm[i].y1 > ncm[i].y2)//if ncm is inverted
						{
							ncm[i].x1 = ncm[i].x1 + (ncm[i].y1 - min(max(ncm[i].y1,ncm[i].y2),gapmi.y2));
							ncm[i].x2 = ncm[i].x2 - (max(min(ncm[i].y1,ncm[i].y2),gapmi.y1) - ncm[i].y2);
							ncm[i].y1 = ncm[i].y1 - (ncm[i].y1 - min(max(ncm[i].y1,ncm[i].y2),gapmi.y2));
							ncm[i].y2 = ncm[i].y2 + (max(min(ncm[i].y1,ncm[i].y2),gapmi.y1) - ncm[i].y2);
						}
						ncm[i].c = 'q';
						ncm[i].l = qOvl;
						smum.push_back(ncm[i]);
						ncm[i].y2 = 0;
						ncm[i].y1 = 0;
						ncm[i].x1 = 0;
						ncm[i].x2 = 0;
					}
				}
			}
		}
//cout<<"2 gap\t"<<gapmi.rn<<'\t'<<gapmi.x1<<'\t'<<gapmi.x2<<'\t'<<gapmi.qn<<'\t'<<gapmi.y1<<'\t'<<gapmi.y2<<endl;
		if((smum.size() >0))
		{
			tempmi = findClosest(gapmi,smum);
			if(find(cnv.begin(),cnv.end(),tempmi) == cnv.end())//tempmi is not present in cnv
			{
				//cnv.push_back(tempmi);
				vd = getCoverage(tempmi,masterRef,masterQ,0.3);
//cout<<"CNV/TEMP\t"<<tempmi.rn<<'\t'<<tempmi.x1<<'\t'<<tempmi.x2<<'\t'<<tempmi.qn<<'\t'<<tempmi.y1<<'\t'<<tempmi.y2<<'\t'<<tempmi.c<<endl;
		//		vn = getChromCount(tempmi,chromDensityRef,chromDensityQ);
//HOW TO ANNOTATE tandem dup: if the dup in ref or query is next MUM from the beginning of the gap
				if(nearestInt(vd[0]) >2 || nearestInt(vd[1]) >2)
				{
					if(tempmi.c == 'r')
					{
						fout<<tempmi.rn<<"\t"<<tempmi.x1<<"\t"<<tempmi.x2<<"\tnCNV-R\t"<<tempmi.qn<<"\t"<<tempmi.y1<<"\t"<<tempmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<tempmi.x2-tempmi.x1<<"\t"<<vd[0]<<"\t"<<vd[1]<<endl;
					}
					if(tempmi.c == 'q')
					{
						fout<<tempmi.rn<<"\t"<<tempmi.x1<<"\t"<<tempmi.x2<<"\tnCNV-Q\t"<<tempmi.qn<<"\t"<<tempmi.y1<<"\t"<<tempmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<tempmi.x2-tempmi.x1<<"\t"<<vd[0]<<"\t"<<vd[1]<<endl;
					}
				}
				else //coverage less than 2
				{
					if(tempmi.c == 'r')
					{
						fout<<tempmi.rn<<"\t"<<tempmi.x1<<"\t"<<tempmi.x2<<"\tCNV-R\t"<<tempmi.qn<<"\t"<<tempmi.y1<<"\t"<<tempmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<tempmi.x2-tempmi.x1<<"\t"<<vd[0]<<"\t"<<vd[1]<<endl;
					}
					if(tempmi.c == 'q')
					{
						fout<<tempmi.rn<<"\t"<<tempmi.x1<<"\t"<<tempmi.x2<<"\tCNV-Q\t"<<tempmi.qn<<"\t"<<tempmi.y1<<"\t"<<tempmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<tempmi.x2-tempmi.x1<<"\t"<<vd[0]<<"\t"<<vd[1]<<endl;
					}
				}
				cnv.push_back(tempmi);
				if(tempmi.c == 'r'||tempmi.c == 'q') //both can also happen
				{
					gapRight = gapmi; //asign the original gap and then the specific ones
					gapLeft = gapmi; //are changed in the following if statements
				}
				if(tempmi.c == 'r')//if the cnv is in reference
				{
					gapRight.x1 = min(tempmi.x2+1,gapmi.x2); //adjust the gap coordinates
					gapRight.x2 = gapmi.x2;						
					gapLeft.x1 = gapmi.x1;
					gapLeft.x2 = max(tempmi.x1,gapmi.x1);
				}
				if(tempmi.c == 'q')
				{
					gapRight.y1 = min(max(tempmi.y2+1,tempmi.y1),gapmi.y2); //adjust the gap coordinates
					gapRight.y2 = gapmi.y2;
					gapLeft.y1 = gapmi.y1;
					gapLeft.y2 = max(min(tempmi.y1,tempmi.y2+1),gapmi.y1);
				}
				findCnvOverlap(gapLeft,smum,cnv,indels,masterRef,masterQ,chromDensityRef,chromDensityQ,fout,id);
				findCnvOverlap(gapRight,smum,cnv,indels,masterRef,masterQ,chromDensityRef,chromDensityQ,fout,id);
			}//end of cnv present or absent condition
			else//cnv present
			{
		//		cout<<"CNV\t"<<tempmi.rn<<'\t'<<tempmi.x1<<'\t'<<tempmi.x2<<'\t'<<tempmi.qn<<'\t'<<tempmi.y1<<'\t'<<tempmi.y2<<endl;
				gapmi.x1 = gapmi.x2;//this will not be used for reporting indels
				gapmi.y1 = gapmi.y2;//this will not be used for reporting indels
			}
		}//end of smum size condition
		else//if no ncm is found then it is a gap in the reference or query
		{
			vd = getCoverage(gapmi,masterRef,masterQ);
			if((gapmi.y2 - gapmi.y1) > (gapmi.x2-gapmi.x1))//insertion
			{
				if(chkIndel(gapmi,indels) == false)//if the indel isn't present already
				{
					if(gapmi.x2 - gapmi.x1 < 0)//overlapping MUMs by reference
					{
						gapmi.x2 = gapmi.x1;
					}
					if(gapmi.y2 - gapmi.y1 < 0)//overlapping MUMs by query
					{
						gapmi.y2 = gapmi.y1;
					}
					fout<<gapmi.rn<<"\t"<<gapmi.x1 <<"\t"<<gapmi.x2<<"\tINS\t"<<gapmi.qn<<"\t"<<gapmi.y1<<"\t"<<gapmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<abs(gapmi.y1-gapmi.y2)-abs(gapmi.x1-gapmi.x1)<<"\t"<<vd[0]<<"\t"<<vd[1]<<endl;	
					indels.push_back(gapmi);
				}
			}
			if((gapmi.x2-gapmi.x1) > (gapmi.y2 - gapmi.y1))
			{
				if(chkIndel(gapmi,indels) == false)//if the indels isn't present already
				{
					if(gapmi.x2 - gapmi.x1 < 0)//overlapping MUMs by reference
					{
						gapmi.x2 = gapmi.x1;
					}
					if(gapmi.y2 - gapmi.y1 < 0)//overlapping MUMs by query
					{
						gapmi.y2 = gapmi.y1;
					}
					fout<<gapmi.rn<<"\t"<<gapmi.x1 <<"\t"<<gapmi.x2<<"\tDEL\t"<<gapmi.qn<<"\t"<<gapmi.y1<<"\t"<<gapmi.y2<<"\t"<<setfill('0')<<setw(10)<<id++<<"\t"<<abs(gapmi.x2-gapmi.x1)-abs(gapmi.y2-gapmi.y1)<<"\t"<<vd[0]<<"\t"<<vd[1]<<endl;
					indels.push_back(gapmi);
				}
			}
		}		
	}

}
////////////////////////////////////////////////////////////////////////
mI findDupRef(mI & mi1, mI & mi2)//Ref is duplicated in query.mi1 is upstream MUM and mi2 is downstream MUM
{
	mI tempmi;
	tempmi.x1= 0;//to use it as a filter for later
	int sharedAln = 0;//shared alignment overlap between ref and query
	if((mi2.x1 < mi1.x2) && !(mi1 == mi2)) //i.e. mi2.x1 falls within the previous interval. this works because mums are sorted by reference cord.
	{
		if(mi2.x2 > mi1.x2) //mi1 amd mi2 refs just overlaps but one is NOT contained within another
		{
			tempmi.rn = mi1.rn;
			//tempmi.x1 = mi2.x1;
			tempmi.qn = mi1.qn;
			tempmi.x2 = min(mi1.x2,mi2.x2); //smallest of the two
			if(mi2.y1 <mi2.y2) //forward oriented
			{
				sharedAln = max((mi1.y2 - mi2.y1),0);//if q doesn't overlap,the second would be <0
				if(mi2.x1 + sharedAln < mi1.x2)//if dup in query and not ref
				{
					tempmi.x1 = mi2.x1 + sharedAln;
					tempmi.y1 = mi2.y1 + sharedAln;
					tempmi.y2 = mi2.y1 + (mi1.x2 - mi2.x1);
				}
			}
			if(mi2.y1 > mi2.y2)//reverse oriented
			{
				if(mi1.y1>mi2.y1) //first is bigger than second
				{
					sharedAln = max(( mi2.y1 -mi1.y2),0);
				}
				if(mi1.y1<mi2.y1)
				{
					sharedAln = max((mi1.y1 - mi2.y2),0);
				}
				if(mi2.x1 + sharedAln < mi1.x2)//if dup in query and not ref
				{
					tempmi.x1 = mi2.x1 + sharedAln;
					tempmi.y2 = mi2.y1 - sharedAln;//second
					tempmi.y1 = mi2.y1 - (mi1.x2 - mi2.x1);
				}
				if((mi1.y1 < mi1.y2) && (mi1.y2-mi2.y2 < mi1.x2-mi2.x1))  //the first mum is NOT inverted and there is larger overlap between reference intervals
				{
					tempmi.x1 = mi2.x1 + abs(mi1.y2-mi2.y2);
					tempmi.x2 = mi1.x2;
					tempmi.y1 = mi2.y2 + abs(mi1.y2-mi2.y2); 
					tempmi.y2 = tempmi.y1 + abs(tempmi.x2-tempmi.x1);
				}
			}
//cout<<"cnv "<<tempmi.rn<<" "<<tempmi.x1<<" "<<tempmi.x2<<" "<<tempmi.qn<<" "<<tempmi.y1<<" "<<tempmi.y2<<endl;
		}
		if(!(mi2.x2>mi1.x2))//mi2 is contained within mi1
		{
			tempmi.rn = mi1.rn;
			tempmi.qn = mi1.qn;
			tempmi.x2 = min(mi1.x2,mi2.x2);
			tempmi.y2 = mi2.y2;
			if(mi2.y1 <mi2.y2) //forward oriented
			{
				sharedAln = max((mi1.y2-mi2.y1),0);
				if(mi2.x1 + sharedAln < mi1.x2)
				{
					tempmi.x1 = mi2.x1 + sharedAln;
					tempmi.y1 = mi2.y1 + sharedAln;
				}
			}
			if(mi2.y1 > mi2.y2)//reverse oriented
			{
				sharedAln = max((mi1.y1 - mi2.y2),0);
				if(mi2.x1 + sharedAln < mi1.x2)
				{
					tempmi.x1 = mi2.x1 + sharedAln;
					tempmi.y1 = mi2.y1 - sharedAln;
				}
			}
//cout<<"cnv "<<tempmi.rn<<" "<<tempmi.x1<<" "<<tempmi.x2<<" "<<tempmi.qn<<" "<<tempmi.y1<<" "<<tempmi.y2<<endl;
		}
	}
	return tempmi;
}
/////////////////////////////////////////////////////////////////////////////	
mI findDupQ(mI & m1,mI & m2)//m1 is thismi and m2 is nextmi
{
	mI tempmi;
	tempmi.rn = m1.rn;//just assign the names.
	tempmi.qn = m1.qn;//same as above
	tempmi.x1 = 0;//filter in case a dup is not found
	//tempmi.y1 = m2.y1;//irrespective of what the y2 would be,y1 would always be this
	if((m1.y1<m1.y2) && (m2.y1 < m2.y2))//both forward
	{
		if(!(m2.y1 > m1.y2))
		{
			tempmi.x1 = m2.x1;
			tempmi.x2 = m2.x1 + (min(m1.y2,m2.y2)-m2.y1);//min is used in case m2.y2 is <m1.y2
			tempmi.y1 = m2.y1;
			tempmi.y2 = min(m1.y2,m2.y2);//m1.y1-m2.y2 = length of the dup
		}
//cout<<"1\t"<<m1.rn<<'\t'<<m1.x1<<'\t'<<m1.x2<<'\t'<<m1.qn<<'\t'<<m1.y1<<'\t'<<m1.y2<<'\t'<<m2.rn<<'\t'<<m2.x1<<'\t'<<m2.x2<<'\t'<<m2.qn<<'\t'<<m2.y1<<'\t'<<m2.y2<<'\t'<<tempmi.x1<<'\t'<<tempmi.x2<<'\t'<<tempmi.y1<<'\t'<<tempmi.y2<<endl;
	}
	if((m1.y1 > m1.y2) && (m2.y1 > m2.y2))//both reverse
	{
		if(!(m2.y1 < m1.y2) && (m1.y1 > m2.y1))//the beginning of next is not smaller than the end of this && within same inversion
		{
			tempmi.x1 = m2.x1;
			tempmi.x2 = m2.x1 + (m2.y1 - max(m1.y2,m2.y2));//same as above, but in this case max inst of min
			tempmi.y1 = m2.y1;//we can swap it with the number below to make it forward strand
			tempmi.y2 = max(m1.y2,m2.y2);
//cout<<"2\t"<<m1.rn<<'\t'<<m1.x1<<'\t'<<m1.x2<<'\t'<<m1.qn<<'\t'<<m1.y1<<'\t'<<m1.y2<<m2.rn<<'\t'<<m2.x1<<'\t'<<m2.x2<<'\t'<<m2.qn<<'\t'<<m2.y1<<'\t'<<m2.y2<<endl;
		}
		if(!(m1.y1 < m2.y2) && (m1.y1 < m2.y1))//the inverse of above
		{
			tempmi.x1 = m2.x1;
			tempmi.x2 = m2.x1 + (m1.y1 - max(m2.y2,m1.y2));//same as above, but in this case max inst of min
			tempmi.y1 = m2.y1;//we can swap it with the number below to make it forward strand
			tempmi.y2 = max(m2.y2,m1.y2);
		}
	}
	if((m1.y1 < m1.y2) && (m2.y1 > m2.y2))//this forward,next reverse
	{
		if(!(m2.y2 > m1.y2) && (m1.y2-m2.y2 > m1.x2-m2.x1)) //the query intervals overlap is bigger than reference overlap
		{
			tempmi.x1 = max(m1.x2,m2.x1); - (m1.y2 - m2.y2);
			tempmi.y1 = m2.y2 + max((m1.x2-m2.x1),0);//subtract the ref overlap
			tempmi.y2 = m1.y2;
			tempmi.x2 = tempmi.x1 + abs(tempmi.y2 - tempmi.y1);
//cout<<"3\t"<<m1.rn<<'\t'<<m1.x1<<'\t'<<m1.x2<<'\t'<<m1.qn<<'\t'<<m1.y1<<'\t'<<m1.y2<<m2.rn<<'\t'<<m2.x1<<'\t'<<m2.x2<<'\t'<<m2.qn<<'\t'<<m2.y1<<'\t'<<m2.y2<<endl;
		}
	}
	if((m1.y1 > m1.y2) && (m2.y1 < m2.y2)) //this reverse, next forward
	{
		if(!(m2.y1 > m1.y1))
		{
			tempmi.x1 = m2.x1;
			tempmi.x2 = m2.x1 + m1.y1 - m2.y1;
			tempmi.y1 = m2.y1;
			tempmi.y2 = m1.y1;
//cout<<"4\t"<<m1.rn<<'\t'<<m1.x1<<'\t'<<m1.x2<<'\t'<<m1.qn<<'\t'<<m1.y1<<'\t'<<m1.y2<<m2.rn<<'\t'<<m2.x1<<'\t'<<m2.x2<<'\t'<<m2.qn<<'\t'<<m2.y1<<'\t'<<m2.y2<<endl;
		}
	}
cout<<tempmi.rn<<'\t'<<tempmi.x1<<'\t'<<tempmi.x2<<'\t'<<tempmi.qn<<'\t'<<tempmi.y1<<'\t'<<tempmi.y2<<endl;
	return tempmi;
}	
bool chkIndel(mI & gapmi,vector <mI> & indels)
{
	bool found = false;
	for(unsigned int i=0;i<indels.size();i++)
	{

		if((indels[i].x1 == gapmi.x1) && (indels[i].x2 == gapmi.x2))//when none are same
		{
			found = true;//same reference gap
		}
		if((indels[i].y1 == gapmi.y1) && (indels[i].y2 == gapmi.y2))
		{
			found = true;
		}
		if(found == true)
		{
			break;
		}
	}
	return found;

}
	
