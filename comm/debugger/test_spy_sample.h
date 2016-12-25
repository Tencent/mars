//
//  testspy.h
//  PublicComponent
//
//  Created by Ray on 14-5-13.
//  Copyright (c) 2014年 Tencent. All rights reserved.
//

#ifndef __PublicComponent__testspy__
#define __PublicComponent__testspy__



class Test_Spy_Sample {
  public:
    Test_Spy_Sample();
    ~Test_Spy_Sample();

    void TestFun0();
    void TestFun2()  {__TestFun1(1);}

  private:
    int __TestFun1(int i);

  private:
//    int m_t;
};


#endif /* defined(__PublicComponent__testspy__) */
