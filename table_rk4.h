#pragma once
#include <vector>
#include <cmath>
#include <list>
#include "rk4.h"
#include "DataTransferObj.h"
#include "functions.h"
using std::vector;
using std::pair;
using std::list;


class TableRK4test : public DataTransferObj
/* ����� ��� ������� ����������� � ��������� �� � ������� ��� �������� ������
 * ����� solveWithoutControl - ������������ ��������� ���������� � ������������� ����������� ��� �������� ��������� �����������
 * ����� solveWithControl - ������������ ��������� ���������� � ������������� ����������� � ��������� ��������� �����������
*/
{
public:

    //��������� ������� ��� �������� ��������� �����������.
    //initVals - ��������� �������, h0 - ��� ��������������, func - ������� � ��, B - ������� ����������, Hmax - ������������ ����� �����, Egr - �������� ������ �� �������
    void solveWithoutControl(const QList<double>& initVals, double h0, double B, int Hmax, double Egr = 0.001) override
	{

        double x0 = initVals[0];
        double u0 = initVals[1];

        xi.clear();
        vi.clear();
        resultSteps2.clear();
        diff_vi_v2i.clear();
        olp.clear();
        hi.clear();
        c1.clear();
        c2.clear();
        ui.clear();
        diff_ui_vi.clear();

        xi.push_back(x0);
        vi.push_back(u0);
        generalSolutionU u(x0,u0);
        for (int i = 0; i < Hmax; i++)
        {
            //������� 1 ������� ���, � 2 �������� ����
            pair<double,double> result_simpleTurn = RK4(xi.back(), vi.back(), h0, testFuncDu);
            pair<double,double> result_halfTurn = RK4(xi.back(), vi.back(), (h0 / 2.0), testFuncDu);
            pair<double,double> result_2_halfTurn = RK4(result_halfTurn.first, result_halfTurn.second, (h0 / 2.0), testFuncDu);

            xi.push_back(result_simpleTurn.first); //��������� ������� (Xn, Vn) � ������ ��������� ����������
            vi.push_back(result_simpleTurn.second);
            resultSteps2.push_back(result_2_halfTurn.second);
            diff_vi_v2i.push_back(result_2_halfTurn.second - result_simpleTurn.second);

            double S = (result_2_halfTurn.second - result_simpleTurn.second) / (pow(2, 4) - 1.0);
            olp.push_back(S * pow(2, 4));
            hi.push_back(h0);
            c1.push_back(0);
            c2.push_back(0);
            ui.push_back(u(result_simpleTurn.first));
            diff_ui_vi.push_back(fabs(u(result_simpleTurn.first) - result_simpleTurn.second));


            //��������� ����� �� �������
            if (xi.back() >= (B - Egr) && xi.back() <= B)
            {
                break;
            }
            else if (xi.back() > B)
            {
                xi.pop_back();
                vi.pop_back();
                resultSteps2.pop_back();
                diff_vi_v2i.pop_back();
                olp.pop_back();
                hi.pop_back();
                
                ui.pop_back();
                diff_ui_vi.pop_back();

                double h1 = B - xi.back() - Egr;

                pair<double, double> result_simpleTurn = RK4(xi.back(), vi.back(), h1, testFuncDu);
                pair<double, double> result_halfTurn = RK4(xi.back(), vi.back(), (h1 / 2.0), testFuncDu);
                pair<double, double> result_2_halfTurn = RK4(result_halfTurn.first, result_halfTurn.second, (h1 / 2.0), testFuncDu);

                double S = (result_2_halfTurn.second - result_simpleTurn.second) / (pow(2, 4) - 1.0);

                xi.push_back(result_simpleTurn.first); //���� ����� ��������, �� ���� ���� �� ����� �� ������� �� ��������� Xn = b - Egr
                vi.push_back(result_simpleTurn.second);
                resultSteps2.push_back(result_2_halfTurn.second);
                diff_vi_v2i.push_back(result_2_halfTurn.second - result_simpleTurn.second);
                olp.push_back(S * pow(2, 4));
                hi.push_back(h0);
                
                ui.push_back(u(result_simpleTurn.first));
                diff_ui_vi.push_back(fabs(u(result_simpleTurn.first) - result_simpleTurn.second));

                break;
            }
        }
        xi.pop_front();
        vi.pop_front();
	}

    //��������� ������� � ��������� ��������� �����������.
    //initVals - ��������� �������, h0 - ��� ��������������, func - ������� � ��, B - ������� ����������, Hmax - ������������ ����� �����, E - �����������, Egr - �������� ������ �� �������
    void solveWithControl(const QList<double>& initVals, double h0, double B, int Hmax, double E, double Egr = 0.001) override
    {

        double x0 = initVals[0];
        double u0 = initVals[1];

        xi.clear();
        vi.clear();
        resultSteps2.clear();
        diff_vi_v2i.clear();
        olp.clear();
        hi.clear();
        c1.clear();
        c2.clear();
        ui.clear();
        diff_ui_vi.clear();

        int c1count = 0;
        int c2count = 0;
        double H = h0;

        xi.push_back(x0);
        vi.push_back(u0);
        generalSolutionU u(x0,u0);

        for (int i = 0; i < Hmax; i++)
        {
            //������� 1 ������� ���, � 2 �������� ����
            pair<double, double> result_simpleTurn = RK4(xi.back(), vi.back(), H, testFuncDu);
            pair<double, double> result_halfTurn = RK4(xi.back(), vi.back(), (H / 2.0), testFuncDu);
            pair<double, double> result_2_halfTurn = RK4(result_halfTurn.first, result_halfTurn.second, (H / 2.0), testFuncDu);

            double S = (result_2_halfTurn.second - result_simpleTurn.second) / (pow(2, 4) - 1);

            //�������� ��� ������ ������
            double h0 = H;
            if (fabs(S) < (E / pow(2, 5))) //�������� ������� ������, ��������� � ��������� ���
            {
                H = H * 2.0;
                c2count++;
            }
            else if (fabs(S) >= E) //�������� ������� ��������, ��������� ��� � ��� ���� � �������������
            {
                H = H / 2.0;
                c1count++;
                i--;
                continue;
            }

            xi.push_back(result_simpleTurn.first); //��������� ������� (Xn, Vn) � ������ ��������� ����������
            vi.push_back(result_simpleTurn.second);
            resultSteps2.push_back(result_2_halfTurn.second);
            diff_vi_v2i.push_back(result_2_halfTurn.second - result_simpleTurn.second);
            olp.push_back(S * pow(2, 4));
            hi.push_back(h0);
            c1.push_back(c1count);
            c2.push_back(c2count);
            ui.push_back(u(result_simpleTurn.first));
            diff_ui_vi.push_back(fabs(u(result_simpleTurn.first) - result_simpleTurn.second));


            //��������� ����� �� �������(� ������ ��� ������� ������)
            if (xi.back() >= (B - Egr) && xi.back() <= B)
            {
                break;
            }
            else if (xi.back() > B)
            {
                xi.pop_back();
                vi.pop_back();
                resultSteps2.pop_back();
                diff_vi_v2i.pop_back();
                olp.pop_back();
                hi.pop_back();
                
                ui.pop_back();
                diff_ui_vi.pop_back();

                double h1 = B - xi.back() - Egr;

                pair<double, double> result_simpleTurn = RK4(xi.back(), vi.back(), h1, testFuncDu);
                pair<double, double> result_halfTurn = RK4(xi.back(), vi.back(), (h1 / 2.0), testFuncDu);
                pair<double, double> result_2_halfTurn = RK4(result_halfTurn.first, result_halfTurn.second, (h1 / 2.0), testFuncDu);

                double S = (result_2_halfTurn.second - result_simpleTurn.second) / (pow(2, 4) - 1.0);

                //���� ����� ��������, �� ���� ���� �� ����� �� ������� �� ��������� Xn = b - Egr
                xi.push_back(result_simpleTurn.first);
                vi.push_back(result_simpleTurn.second);
                resultSteps2.push_back(result_2_halfTurn.second);
                diff_vi_v2i.push_back(result_2_halfTurn.second - result_simpleTurn.second);
                olp.push_back(S * pow(2, 4));
                hi.push_back(h0);
                
                ui.push_back(u(result_simpleTurn.first));
                diff_ui_vi.push_back(fabs(u(result_simpleTurn.first) - result_simpleTurn.second));

                break;
            }
        }
        xi.pop_front();
        vi.pop_front();
    }

};

class TableRK4_1task : public DataTransferObj
{
/* ����� ��� ������� ����������� � ��������� �� � ������� ��� �������� ������ ������
 * ����� solveWithoutControl - ������������ ��������� ���������� � ������������� ����������� ��� �������� ��������� �����������
 * ����� solveWithControl - ������������ ��������� ���������� � ������������� ����������� � ��������� ��������� �����������
*/
public:    
    //��������� ������� ��� �������� ��������� �����������.
    //initVals - ��������� �������, h0 - ��� ��������������, func - ������� � ��, B - ������� ����������, Hmax - ������������ ����� �����, Egr - �������� ������ �� �������
    void solveWithoutControl(const QList<double>& initVals, double h0, double B, int Hmax, double Egr = 0.001) override
    {

        double x0 = initVals[0];
        double u0 = initVals[1];

        xi.clear();
        vi.clear();
        resultSteps2.clear();
        diff_vi_v2i.clear();
        olp.clear();
        hi.clear();
        c1.clear();
        c2.clear();

        xi.push_back(x0);
        vi.push_back(u0);
        for (int i = 0; i < Hmax; i++)
        {
            //������� 1 ������� ���, � 2 �������� ����
            pair<double, double> result_simpleTurn = RK4(xi.back(), vi.back(), h0, testFuncDu);
            pair<double, double> result_halfTurn = RK4(xi.back(), vi.back(), (h0 / 2.0), testFuncDu);
            pair<double, double> result_2_halfTurn = RK4(result_halfTurn.first, result_halfTurn.second, (h0 / 2.0), testFuncDu);

            xi.push_back(result_simpleTurn.first); //��������� ������� (Xn, Vn) � ������ ��������� ����������
            vi.push_back(result_simpleTurn.second);
            resultSteps2.push_back(result_2_halfTurn.second);
            diff_vi_v2i.push_back(result_2_halfTurn.second - result_simpleTurn.second);

            double S = (result_2_halfTurn.second - result_simpleTurn.second) / (pow(2, 4) - 1);
            olp.push_back(S * pow(2, 4));
            hi.push_back(h0);
            c1.push_back(0);
            c2.push_back(0);
            


            //��������� ����� �� �������(� ������ ��� ������� ������)
            if (xi.back() >= (B - Egr) && xi.back() <= B)
            {
                break;
            }
            else if (xi.back() > B)
            {
                xi.pop_back();
                vi.pop_back();
                resultSteps2.pop_back();
                diff_vi_v2i.pop_back();
                olp.pop_back();
                hi.pop_back();
                
                double h1 = B - xi.back() - Egr;

                pair<double, double> result_simpleTurn = RK4(xi.back(), vi.back(), h1, testFuncDu);
                pair<double, double> result_halfTurn = RK4(xi.back(), vi.back(), (h1 / 2), testFuncDu);
                pair<double, double> result_2_halfTurn = RK4(result_halfTurn.first, result_halfTurn.second, (h1 / 2), testFuncDu);

                double S = (result_2_halfTurn.second - result_simpleTurn.second) / (pow(2, 4) - 1);

                xi.push_back(result_simpleTurn.first); //���� ����� ��������, �� ���� ���� �� ����� �� ������� �� ��������� Xn = b - Egr
                vi.push_back(result_simpleTurn.second);
                resultSteps2.push_back(result_2_halfTurn.second);
                diff_vi_v2i.push_back(result_2_halfTurn.second - result_simpleTurn.second);
                olp.push_back(S * pow(2, 4));
                hi.push_back(h1);
                
                

                break;
            }
        }
        xi.pop_front();
        vi.pop_front();
    }

    //��������� ������� � ��������� ��������� �����������.
    //initVals - ��������� �������, h0 - ��� ��������������, func - ������� � ��, B - ������� ����������, Hmax - ������������ ����� �����, E - �����������, Egr - �������� ������ �� �������
    void solveWithControl(const QList<double>& initVals, double h0, double B, int Hmax, double E, double Egr = 0.001) override
    {

        double x0 = initVals[0];
        double u0 = initVals[1];

        xi.clear();
        vi.clear();
        resultSteps2.clear();
        diff_vi_v2i.clear();
        olp.clear();
        hi.clear();
        c1.clear();
        c2.clear();
        

        int c1count = 0;
        int c2count = 0;
        double H = h0;
        xi.push_back(x0);
        vi.push_back(u0);

        for (int i = 0; i < Hmax; i++)
        {
            //������� 1 ������� ���, � 2 �������� ����
            pair<double, double> result_simpleTurn = RK4(xi.back(), vi.back(), H, testFuncDu);
            pair<double, double> result_halfTurn = RK4(xi.back(), vi.back(), (H / 2), testFuncDu);
            pair<double, double> result_2_halfTurn = RK4(result_halfTurn.first, result_halfTurn.second, (H / 2), testFuncDu);

            double S = (result_2_halfTurn.second - result_simpleTurn.second) / (pow(2, 4) - 1);

            //�������� ��� ������ ������
            double h0 = H;
            if (fabs(S) < (E / pow(2, 5))) //�������� ������� ������, ��������� � ��������� ���
            {
                H = H * 2;
                c2count++;
            }
            else if(fabs(S) >= E) //�������� ������� ��������, ��������� ��� � ��� ���� � �������������
            {
                H = H / 2;
                c1count++;
                i--;
                continue;
            }

            xi.push_back(result_simpleTurn.first);
            vi.push_back(result_simpleTurn.second);
            resultSteps2.push_back(result_2_halfTurn.second);
            diff_vi_v2i.push_back(result_2_halfTurn.second - result_simpleTurn.second);
            olp.push_back(S * pow(2, 4));
            hi.push_back(h0);
            c1.push_back(c1count);
            c2.push_back(c2count);
            


            //��������� ����� �� �������(� ������ ��� ������� ������)
            if (xi.back() >= (B - Egr) && xi.back() <= B)
            {
                break;
            }
            else if (xi.back() > B)
            {
                xi.pop_back();
                vi.pop_back();
                resultSteps2.pop_back();
                diff_vi_v2i.pop_back();
                olp.pop_back();
                hi.pop_back();
               
               

                double h1 = B - xi.back() - Egr;

                pair<double, double> result_simpleTurn = RK4(xi.back(), vi.back(), h1, testFuncDu);
                pair<double, double> result_halfTurn = RK4(xi.back(), vi.back(), (h1 / 2), testFuncDu);
                pair<double, double> result_2_halfTurn = RK4(result_halfTurn.first, result_halfTurn.second, (h1 / 2), testFuncDu);

                double S = (result_2_halfTurn.second - result_simpleTurn.second) / (pow(2, 4) - 1);

                xi.push_back(result_simpleTurn.first);//���� ����� ��������, �� ���� ���� �� ����� �� ������� �� ��������� Xn = b - Egr
                vi.push_back(result_simpleTurn.second);
                resultSteps2.push_back(result_2_halfTurn.second);
                diff_vi_v2i.push_back(result_2_halfTurn.second - result_simpleTurn.second);
                olp.push_back(S * pow(2, 4));
                hi.push_back(h1);
                
                

                break;
            }
        }
        xi.pop_front();
        vi.pop_front();
    }

};
