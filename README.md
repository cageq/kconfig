# kconfig 

    * 简单的读取配置文件库,只需要包含头文件，支持简单的key/value配置以及支持数组

    * 使用了c++11  编写，支持函数方式读取，读取后直接通过回调函数回调。 

# 配置文件格式

```bash
    # last modified 1 April 2001 by John Doe
    [owner]
    name=John Doe
    organization=Acme Widgets Inc.

    [database]
    # use IP address in case network name resolution is not working
    server=192.0.2.62     
    port=143
    file=     "payroll.dat"
    file1=payroll.dat   dddadfa 
    name=test
    items=  [  11,43,55,55,33]
```

   >支持使用[] 进行分段;
   >支持# 行注释;
   >如果值为数组，请使用[]; 


# 示例

    KConfig gCfg  ; 
    gCfg.read("samples/test.cfg"); // 读取配置文件

    std::cout << "port is " << gCfg.getInt("port")<< std::endl;  // 普通函数方式获取配置数据
    std::cout << "start is " << gCfg.getBool("start")<< std::endl; 

    //如果初始化传入true 表示支持数据段分割. 上文配置中的port 就会需要通过database.port 才能获取  
    KConfig segConfig (true); // 
    segConfig.read("sample/test.cfg"); 


    std::cout << "port is " << segConfig.getInt("database.port")<< std::endl; 
    std::cout << "start is " << segConfig.getBool("database.start")<< std::endl; 
    std::cout << "name is " << segConfig.getString("database.name")<< std::endl; 


    // 通过模版的方式获取数据，模版参数需要指明获取的数据类型
    std::vector<int> keys =  segConfig.getArray<int>("database.items") ; 
    for(auto it:keys)
    {
        std::cout << "item is " << it << std::endl; 
    }

    // 或通过lambda 函数获取配置文件的值

    segConfig.get<int>("database.port",[](const int &rst ){  
       std::cout << " result is "<< rst << std::endl;
    } ) ;                 



