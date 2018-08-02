function asyncFunctionTests() {
    var idata = 0 ;
    var sdata ="";
    var sum = 0;
    var str = "";
    var con;

    api.getIntValueAsync( function (val) {
        idata = val;
    });

    tryVerify(function(){ return idata;},2000);
    compare(idata,10);


    api.getStringValueAsync( function (val) {
        sdata = val;
    });

    tryVerify(function(){ return sdata;},2000);
    compare(sdata,"Test-String");


    api.getContainerValueAsync( function (val) {
        con = val;
    });

    tryVerify(function(){ return con;},2000);
    compare(con.idata,9);
    compare(con.sdata,"Test-String");

    api.calculateSumAsync(1,2,3,function (val){
        sum = val;
    });

    tryVerify(function(){ return sum;},2000);
    compare(sum,6);


    api.append("Deutsch","Land",function (val){
        str = val;
    });

    tryVerify(function(){ return str;},2000);
    compare(str,"DeutschLand");
}
