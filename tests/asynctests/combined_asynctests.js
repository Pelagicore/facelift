
function testAll() 
{

    api.getIntValueAsync( function (val) {
            compare(val,10);
            });

    api.getStringValueAsync( function (val) {
            compare(val,"Test-String" );
            });

    
    api.getContainerValueAsync( function (val) {
            compare(val.idata,9);
            compare(val.sdata,"Test-String");
            // compare(val.mdata["Test-String"],27);
            });



    // asynchronous call
    api.calculateSumAsync(1,2,3,function (val){
        compare(6,val);
    });


    api.append("Rakesh","Sadhu",function (val){
        compare(val,"RakeshSadhu");
    });





    //synchronous call 
    //compare(api.calculateSum(1,2,3),6);
    //compare(api.value,0);

   // compare(api.compareEnums(AsyncEnums.EASYNC0,AsyncEnums.EASYNC1),AsyncEnums.EASYNC1);
   // compare(api.compareEnums(AsyncEnums.EASYNC0,AsyncEnums.EASYNC0),AsyncEnums.EASYNC0);
   
}

