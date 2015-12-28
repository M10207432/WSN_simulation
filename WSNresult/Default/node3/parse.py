import os

def parsefile(path,output):
    rate=40
    data=[]
    f=open(path,'r')
    for i in f:
        if i.find("SetAmount_MeetRatio")>=0:
            o=i[i.find('=')+1:i.find('\n')]
            data.append(o)
            
        if i.find("Missratio_perset")>=0:
            o=i[i.find('=')+1:i.find('\n')]
            if o.find('-')<0:
                data.append(o)
            else:
                data.append("0")

        if i.find("Meetratio_perset")>=0:
            o=i[i.find('=')+1:i.find('\n')]
            if o.find('-')<0:
                data.append(o)
            else:
                data.append("0")

        if i.find("MissLatency_perpkt")>=0:
            o=i[i.find('=')+1:i.find('\n')]
            if o.find('-')<0:
                data.append(o)
            else:
                data.append("0")

        if i.find("MeetLatency_perpkt")>=0:
            data.append(i[i.find('=')+1:i.find('\n')])

        if i.find("Lifetime")>=0:
            data.append(i[i.find('=')+1:i.find('\n')])

        if i.find("AverageEnergy")>=0:
            data.append(i[i.find('=')+1:i.find('\n')])
            
        if i.find("==============================================")>=0:
            print rate,data[0],data[1],data[2]
            outputdata=str(rate)+','+data[0]+','+data[1]+','+data[2]+','+data[3]+','+data[4]+','+data[5]+','+data[6]+'\n'
            output.write(outputdata)
            data=[]
            rate+=40
    f.close()
    
def pltfile(filename, target, outfilename):

    t_index=0
    state=0

    dic={}
    name=0
    with open(filename, 'r') as f:
        for l in f:
            data=l.split(",")

            #Find the name
            if len(data)<5:
                name=l[:l.find(',')]
                #print name
                dic[name]=[]
                state=1
                
            #Find the data index
            if l.find(target)>=0 and state==1:
                t_index=data.index(target)
                state=2
                
            #Find the value
            if len(data)>=t_index+1 and state==2 and l.find(target)==-1:
                dic[name].append(data[t_index])
                #print data[t_index]
    '''=========================================
                dic[name][0->data len]
    ========================================='''
    rate=40
    pltname=outfilename
    plt=open(pltname,'w')
    pltname=''
    for l in dic:
        pltname=pltname+l+" "
    print pltname
    data=pltname.split(' ')
    datalen=len(dic[data[0]])
    
    plt.write("Rate "+pltname+'\n')
    
    for i in range (0,datalen):
        if rate <= 480:
            plt.write(str(rate)+" ")
            for key in dic:
                print dic[key][i],
                data=dic[key][i]+" "
                plt.write(data)
            plt.write('\n')
            rate=rate+40
            print ""
    plt.close()
    
def main():
    
    filename="Result.csv"

    
    #Write file
    output=open(filename,'w')
    
    Exlist=os.listdir('.')
    for l in Exlist:
        if l.find('.')<0:
            path='./'+l+'/FinalResult.txt'
            print l
            output.write(l+'\n')
            output.write(",SetAmount_MeetRatio,Missratio_perset,Meetratio_perset,MissLatency_perpkt,MeetLatency_perpkt,Lifetime,AverageEnergy"+'\n')
            if l.find('.py')<0 and l.find('.txt')<0:
                parsefile(path,output)
    output.close()
    
    
    #Write plt format
    pltfile(filename,"Meetratio_perset","MeetRatio.txt")
    pltfile(filename,"Lifetime","Lifetime.txt")  
    
    
if __name__=="__main__":
    main()
