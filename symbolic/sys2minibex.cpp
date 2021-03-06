#include <iostream>
#include <stdlib.h>
#include <ginac/ginac.h>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
//#include "ibex.h"

#define VAR "variables"
#define CST "constraint"
#define EXP "define"
#define RTH "routh"

// ginac symbolic based tools for SISO transfert function modulus computation. Exportation to minibex text file handle.

using namespace std;
using namespace GiNaC;



//bool syscst2minibex(string filename,vector<Function*> *ibex_csts) {
bool syscst2minibex(char * filename) {
    symtab table,tablerouth;
    realsymbol w("w");
    ex s = I*w;
    table["s"] = s;
    symbol p("s");
    tablerouth["s"] = p; // routh must be expressed as a polynomial on s, must not use s = iw;
    vector< pair<string,ex> > csts;
    ex ftbf;
    string line;
    unsigned int i(0);
    string varex;
    ifstream myfile;
    vector<string> varexps;
    myfile.open(filename);
    if (!myfile.is_open()) {
        cout<<"error: cannot open file: "<<filename<<endl;
        return false;
    }
    while(getline (myfile,line))
    {
        cout<<"current line: "<<endl<<line<<endl;
        i++;
        if(line.empty()){
            continue;
        }
        else if(line.at(0) != '#') {
            cout<<"error at line "<<i<<": expected declaration (use #)"<<endl;
            return false;
        }
        else {
            if(line.find(VAR)!=string::npos) {
                cout<<"find variables expression"<<endl;
                if(!getline(myfile,line)) {
                    cout<<"error: cannot read line "<<i+1<<endl;
                    return false;
                }
                size_t found = line.find(',');
                size_t fprev = 0;
                bool stop(false);
                while(!stop) {
                    if(found==string::npos) {
                        varex = line.substr(fprev);
                        stop = true;
                    }
                    varex = line.substr(fprev,found-fprev);
                    cout<<"add symbol "<<varex<<endl;
                    table[varex] = realsymbol(varex);
                    tablerouth[varex] = realsymbol(varex);
                    varexps.push_back(varex);
                    fprev = found+1;
                    found = line.find(',',found+1);
                }
                i++;
            }
            else if(line.find(EXP)!=string::npos) {
                cout<<"definition found"<<endl;
                string symb = line.substr(8);
                cout<<"symbol: "<<symb<<endl;
                if(!getline(myfile,line)) {
                    cout<<"error: cannot read line "<<i+1<<endl;
                    return false;
                }
                parser reader(table);
                ex tmp = reader(line);
                cout<<"expression: "<<tmp<<endl;
                table[symb] = tmp;

                parser readerouth(tablerouth);
                ex tmprouth = readerouth(line);
                tablerouth[symb] = tmprouth;

                i++;
            }
            else if(line.find(CST)!=string::npos) {
                string symbol = line.substr(12);
                cout<<"constraint found: "<<symbol<<endl;
                if(!getline(myfile,line)) {
                    cout<<"error: cannot read line "<<i+1<<endl;
                    return false;
                }
                parser reader(table);
                ex tmp = reader(line);
                cout<<"cst expression: "<<tmp<<endl;
                csts.push_back(pair<string,ex>(symbol,tmp));
                i++;
            }
            else if(line.find(RTH)!=string::npos) {
                if(!getline(myfile,line)) {
                    cout<<"error: cannot read line "<<i+1<<endl;
                    return false;
                }
                parser reader(tablerouth);
                ftbf = reader(line);
                cout<<"FTBF: "<<ftbf<<endl;
                i++;
            }
        }
    }
    myfile.close();
    ofstream out;
    // write constraints as minibex function
    while(!csts.empty()) {
        // compute modulus...
        cout<<"cst expression: "<<csts.back().second<<endl;
        ex cst = csts.back().second;
        ex num1 = numer(cst.expand());
        ex numD = pow(real_part(num1),2)+pow(imag_part(num1),2);
        ex den1 = denom(cst.expand());
        ex denD = pow(real_part(den1),2)+pow(imag_part(den1),2);
        ex f_modulus = numD/denD;

//        numD = collect_common_factors(numD);
//        denD = collect_common_factors(denD);

        //replace w by exp(u)
        stringstream ss;
        ss<<f_modulus;
//        ss<<numD<<"/"<<denD;
        string mod_ex = ss.str();
//        regex e("(w)(\\^)(\\d*)");
//        regex_replace (mod_ex,e,"EX");

        // write in txt file to be read by minibex
        string outname = csts.back().first;
        outname.append(".txt");
        out.open(outname.c_str());
        if(!out.is_open()) {
            cout<<"cannot open "<<outname<<endl;
        }
        out<<"function f(";
        for(unsigned i=0;i<varexps.size();i++)
        {
            out<<varexps.at(i)<<",";
        }
        out<<"u)"<<endl;
        out<<" return("<<mod_ex<<");"<<endl;
        out<<"end";
        out.close();
        csts.pop_back();
    }
    // write routh criterion as minibex function
    ex denftbf = denom(ftbf);
    int deg = denftbf.degree(p);
    out.open("routh.txt");
    if(!out.is_open()) {
        cout<<"cannot open "<<"routh.txt"<<endl;
    }
    out<<"function f(";
    for(unsigned i=0;i<varexps.size()-1;i++)
    {
        out<<varexps.at(i)<<",";
    }
    out<<varexps.back();
    out<<")"<<endl;
//    if(deg==2) {
//        out<<"return(";
//        for(unsigned i=0;i<=2;i++)
//            out<<denftbf.coeff(p,i)<<",";
//        out<<denftbf.coeff(p,1)<<","<<denftbf.coeff(p,0)<<")";
//    }
//    if(deg==3) {
//        out<<"return(";
//        for(unsigned i=0;i<=3;i++)
//            out<<denftbf.coeff(p,i)<<",";
//        out<<denftbf.coeff(p,2)*denftbf.coeff(p,1)-denftbf.coeff(p,3)*denftbf.coeff(p,0)<<")";
//    }
//    if(deg==4) {
//        cout<<"degree of polynomial "<<denftbf<<" : "<<deg<<endl;
//        out<<"return(";
//        for(unsigned i=0;i<=4;i++)
//            out<<denftbf.coeff(p,i)<<",";
//        out<<denftbf.coeff(p,3)*denftbf.coeff(p,2)-denftbf.coeff(p,4)*denftbf.coeff(p,1)<<","<<
//             denftbf.coeff(p,3)*denftbf.coeff(p,2)*denftbf.coeff(p,1)-denftbf.coeff(p,4)*denftbf.coeff(p,1)*denftbf.coeff(p,1)-
//             denftbf.coeff(p,3)*denftbf.coeff(p,3)*denftbf.coeff(p,1)<<")";
//    }

//    else
//    {
        cout<<"degree of polynomial "<<denftbf<<" : "<<deg<<endl;
        int nbcol = (deg+1)%2==1?(deg+2)/2:(deg+1)/2;
        cout<<"nbcol: "<<nbcol<<endl;
        ex routhtable[deg+1][nbcol];
        cout<<"size of routhtable: "<<deg+1<<","<<nbcol<<endl;
        for(unsigned i=0;i<=deg;i+=2) {
            routhtable[0][i/2] = denftbf.coeff(p,deg-i);
            cout<<"routhtable 0,"<<i/2<<": "<<denftbf.coeff(p,deg-i)<<endl;
            routhtable[1][i/2] = denftbf.coeff(p,deg-(i+1));
            cout<<"routhtable 1,"<<i/2<<": "<<denftbf.coeff(p,deg-(i+1))<<endl;
        }
        out<<"return("<<routhtable[0][0]<<","<<routhtable[1][0];
        for(unsigned i=2;i<=deg;i++)
        {
            for(unsigned j=0;j<nbcol-1;j++){
                cout<<"indice: "<<i<<","<<j<<endl;
                if(routhtable[i-2][j+1] == 0 || routhtable[i-1][0] ==0 )
                    routhtable[i][j] = 0;
                else{
                    routhtable[i][j] = -1/routhtable[i-1][0]*(routhtable[i-2][0]*routhtable[i-1][j+1]-routhtable[i-1][0]*routhtable[i-2][j+1]);
                    routhtable[i][j] = routhtable[i][j].normal();
                }
                cout<<routhtable[i][j]<<endl;
            }
//            ex tmp = numer_denom(routhtable[i][0].expand());
            out<<","<<routhtable[i][0];//<<"/"<<routhtable[i][j];
        }
        out<<")";
//    }
    out<<endl<<"end";
    out.close();
    cout<<"denom: "<<denftbf<<endl;
    return true;
}

int main(int argc,char *argv[]) {
    cout<<"try to open file: "<<argv[1]<<endl;
    return syscst2minibex(argv[1]);

}
