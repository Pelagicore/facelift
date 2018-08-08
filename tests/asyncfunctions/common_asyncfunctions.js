
function commonCode(){

    var idata = 0 , sdata ="", sum = 0, str = "", con;

   // 1st case
    
    api.getIntValueAsync( function (val) {
        idata = val;
     });

    tryVerify(function(){ return idata;},2000);
    compare(idata,10);

   
   // 2nd case
    
    api.getStringValueAsync( function (val) {
        sdata = val;
    });

    tryVerify(function(){ return sdata;},2000);
    compare(sdata,"Test-String");

    // 3rd case
    
    api.getContainerValueAsync( function (val) {
        con = val;
    });

    tryVerify(function(){ return con;},2000);
    compare(con.idata,9);
    compare(con.sdata,"Test-String");


    //4th case

    api.calculateSumAsync(1,2,3,function (val){
        sum = val;
    });
    
    tryVerify(function(){ return sum;},2000);
    compare(sum,6);

    //5th case
    
    api.append("Rakesh","Sadhu",function (val){
        str = val;
    });
    
    tryVerify(function(){ return str;},2000);
    compare(str,"RakeshSadhu");
}
