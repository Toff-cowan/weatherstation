 #!/usr/bin/python3


#################################################################################################################################################
#                                                    CLASSES CONTAINING ALL THE APP FUNCTIONS                                                                                                    #
#################################################################################################################################################


class DB:

    def __init__(self,Config):

        from math import floor
        from os import getcwd
        from os.path import join
        from json import loads, dumps, dump
        from datetime import timedelta, datetime, timezone 
        from pymongo import MongoClient , errors, ReturnDocument
        from urllib import parse
        from urllib.request import  urlopen 
        from bson.objectid import ObjectId  
       
      
        self.Config                         = Config
        self.getcwd                         = getcwd
        self.join                           = join 
        self.floor                      	= floor 
        self.loads                      	= loads
        self.dumps                      	= dumps
        self.dump                       	= dump  
        self.datetime                       = datetime
        self.ObjectId                       = ObjectId 
        self.server			                = Config.DB_SERVER
        self.port			                = Config.DB_PORT
        self.username                   	= parse.quote_plus(Config.DB_USERNAME)
        self.password                   	= parse.quote_plus(Config.DB_PASSWORD)
        self.remoteMongo                	= MongoClient
        self.ReturnDocument                 = ReturnDocument
        self.PyMongoError               	= errors.PyMongoError
        self.BulkWriteError             	= errors.BulkWriteError  
        self.tls                            = False # MUST SET TO TRUE IN PRODUCTION


    def __del__(self):
            # Delete class instance to free resources
            pass
 


    ####################
    # LAB 2 DATABASE UTIL FUNCTIONS  #
    ####################
    
    def addUpdate(self,data):
        '''ADD A NEW STORAGE LOCATION TO COLLECTION'''
        try:
            remotedb 	= self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password,self.server,self.port), tls=self.tls)
            result      = remotedb.ELET2415.weather.insert_one(data)
        except Exception as e:
            msg = str(e)
            if "duplicate" not in msg:
                print("addUpdate error ",msg)
            return False
        else:                  
            return True
        
       

    def getAllInRange(self,start, end):
        '''RETURNS A LIST OF OBJECTS. THAT FALLS WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb 	= self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password,self.server,self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.find({'timestamp':{'$gte':int(start),'$lte':int(end)}}, {'_id':0}).sort('timestamp',1))
        except Exception as e:
            msg = str(e)
            print("getAllInRange error ",msg)            
        else:                  
            return result
        

    def humidityMMAR(self,start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR HUMIDITY. THAT FALLS WITHIN THE START AND END DATE RANGE'''
        try:
            
            remotedb 	= self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password,self.server,self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.aggregate([{'$match': {'timestamp':{'$gte':int(start),'$lte':int(end)}}}, {'$group': {'_id': None, 'humidity': {'$push': '$$ROOT.humidity'}}}, {'$project': {"_id": 0, 'min': {'$min': '$humidity'}, 'max': {'$max': '$humidity'}, 'avg': {'$avg': '$humidity'}, 'range': {'$subtract': [{'$max': '$humidity'}, {'$min': '$humidity'}]}}}]))
        except Exception as e:
            msg = str(e)
            print("humidityMMAR error ",msg)
        else:
            return result       
        
        
    def temperatureMMAR(self,start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR TEMPERATURE. THAT FALLS WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb 	= self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password,self.server,self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.weather.aggregate([{'$match': {'timestamp':{'$gte':int(start),'$lte':int(end)}}}, {'$group': {'_id': None, 'temperature': {'$push': '$$ROOT.temperature'}}}, {'$project': {"_id": 0, 'min': {'$min': '$temperature'}, 'max': {'$max': '$temperature'}, 'avg': {'$avg': '$temperature'}, 'range': {'$subtract': [{'$max': '$temperature'}, {'$min': '$temperature'}]}}}]))
        except Exception as e:
            msg = str(e)
            print("temperatureMMAS error ",msg)            
        else:                  
            return result

    def frequencyDistro(self,variable,start, end):
        '''RETURNS THE FREQUENCY DISTROBUTION FOR A SPECIFIED VARIABLE WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb 	= self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password,self.server,self.port), tls=self.tls)
            result      = list(remotedb.ELET2415.climo.aggregate([{"$match": {"timestamp": {"$gte": int(start), "$lte": int(end)}}}, {"$bucket": {"groupBy": "$" + variable, "boundaries": [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100], "default": "outliers", "output": {"count": {"$sum": 1}}}}]))
        except Exception as e:
            msg = str(e)
            print("frequencyDistro error ",msg)            
        else:                  
            return result
        
    def pressureMMAR(self, start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR PRESSURE WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result = list(remotedb.ELET2415.weather.aggregate([{'$match': {'timestamp': {'$gte': int(start), '$lte': int(end)}}},{'$group': {'_id': None, 'pressure': {'$push': '$$ROOT.pressure'}}},{'$project': {'_id': 0, 'min': {'$min': '$pressure'}, 'max': {'$max': '$pressure'}, 'avg': {'$avg': '$pressure'}, 'range': {'$subtract': [{'$max': '$pressure'}, {'$min': '$pressure'}]}}}]))
        except Exception as e:
            print("pressureMMAR error", str(e))
        else:
            return result
    
    def heatIndexMMAR(self, start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR HEAT INDEX WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result = list(remotedb.ELET2415.weather.aggregate([
                {'$match': {'timestamp': {'$gte': int(start), '$lte': int(end)}}},
                {'$group': {'_id': None, 'heat_index': {'$push': '$$ROOT.heat_index'}}},
                {'$project': {'_id': 0, 'min': {'$min': '$heat_index'}, 'max': {'$max': '$heat_index'}, 'avg': {'$avg': '$heat_index'}, 'range': {'$subtract': [{'$max': '$heat_index'}, {'$min': '$heat_index'}]}}}
            ]))
        except Exception as e:
            print("heatIndexMMAR error", str(e))
        else:
            return result
    
    def altitudeMMAR(self, start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR ALTITUDE WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result = list(remotedb.ELET2415.weather.aggregate([{'$match': {'timestamp': {'$gte': int(start), '$lte': int(end)}}},{'$group': {'_id': None, 'altitude': {'$push': '$$ROOT.altitude'}}},{'$project': {'_id': 0, 'min': {'$min': '$altitude'}, 'max': {'$max': '$altitude'}, 'avg': {'$avg': '$altitude'}, 'range': {'$subtract': [{'$max': '$altitude'}, {'$min': '$altitude'}]}}}
            ]))
        except Exception as e:
            print("altitudeMMAR error", str(e))
        else:
            return result
    
    def moistureMMAR(self, start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR MOISTURE WITHIN THE START AND END DATE RANGE'''
        try:
            remotedb = self.remoteMongo('mongodb://%s:%s@%s:%s' % (self.username, self.password, self.server, self.port), tls=self.tls)
            result = list(remotedb.ELET2415.weather.aggregate([{'$match': {'timestamp': {'$gte': int(start), '$lte': int(end)}}},{'$group': {'_id': None, 'moisture': {'$push': '$$ROOT.moisture'}}},{'$project': {'_id': 0, 'min': {'$min': '$moisture'}, 'max': {'$max': '$moisture'}, 'avg': {'$avg': '$moisture'}, 'range': {'$subtract': [{'$max': '$moisture'}, {'$min': '$moisture'}]}}}]))
        except Exception as e:
            print("moistureMMAR error", str(e))
        else:
            return result

 



def main():
    from config import Config
    from time import time, ctime, sleep
    from math import floor
    from datetime import datetime, timedelta
    one = DB(Config)
 
 
    start = time() 
    end = time()
    print(f"completed in: {end - start} seconds")
    
if __name__ == '__main__':
    main()


    
