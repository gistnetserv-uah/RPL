#! /usr/bin/python
# -*- coding: utf-8 -*-

#Python 3 script :)

import os
import sys  
import operator
import argparse
from collections import OrderedDict

#Main fuction of the parser 
def hopscount_parser(base_name):

    #Dir, files, and aux vars
    result_dir = 'Parsed_data'
    path = os.getcwd()+'/'+base_name
    files = os.listdir(path)
    sim_conv = False
    sim_stats = {'sim_total':'0','sim_total_conv':'0',
                 'sim_total_no_conv':'0','sim_total_recovered':'0',
                 'sim_total_no_recovered':'0',
                 'sim_total_no_conv':'0',
                 'sim_total_conv':'0',
                 'sim_total_seeds':[],
                 'sim_total_conv_seeds':[],
                 'sim_total_no_conv_seeds':[],
                 'sim_total_recovered_seeds':[],
                 'sim_total_no_recovered_seeds':[]
                 }


    #Create a final dir where it will include parse data
    os.mkdir(path+'/'+result_dir+'/')

    #Param. for the parser -> -ns (non-storing) -s (Storing)
    parser = argparse.ArgumentParser()
    parser.add_argument("strings", metavar="<dir>", type=str, help ="Dir to parse")
    parser.add_argument("-ns", "--nonstoring", help="Non Storing Parsing", action="store_true")
    parser.add_argument("-s", "--storing", help="Storing Parsing", action="store_true")
    args = parser.parse_args()

    #To log all the operations during the parsing
    try:
        log_parser = open(path +'/.hopscount_parser.log', 'w+')
        if args.storing:
            log_parser.write('Mode: Storing activated\n\n')
        elif args.nonstoring:
            log_parser.write('Mode: Non-Storing activated\n\n')
    except:
        print('Error: cannot create the parser logs\n')
    
    #We only want to parse the .scritplog generated by cooja
    for file in files:
        if(file != result_dir):
            if(file.count('scriptlog')):

                try:
                    # Prepare to read
                    file_to_parse = open(path+'/'+file, 'r')
                    log_parser.write('\n\nINFO: Reading the file '+file +'\n')
                    # Stats
                    sim_stats['sim_total'] = str(int( sim_stats['sim_total']) + 1)
                    sim_stats['sim_total_seeds'].append(get_random_seed(file_to_parse))

                except:
                    print('Error, cannot read the file: '+file)

                try:
                    # Prepare to write
                    file_parsed = open(path+'/'+result_dir+'/parsed_'+file,'w')
                    log_parser.write('INFO: Creating the file parsed_'+file+'\n')
                except:
                    print('Error, cannot create the file: parsed_'+file)

                try:
                    #Count the number of motes
                    num_motes=motes_count_per_file(file_to_parse)
                    log_parser.write('INFO: Counting each mote '+'\n')
                except:
                    print('Error, cannot read the number of motes in the file: '+ file)
               
                try:
                    #Recover all ipv6's and parse them to local-ipv6
                    dic_motes_ip = read_ip_modes(file_to_parse)
                    log_parser.write('INFO: Creating data struct for all IPs in the file:' + file +'\n')
                except:
                    print('Error, cannot read motes IP in the file: '+ file)
                
                try:
                    #Create data struct for hops, all with all
                    dic_motes = {0:{'Nothing':'here'}}
                    for i in range(1,num_motes+1):
                        dic_motes[i]  = {}
                        for j in range(1,(num_motes+1)):
                            if(j != i):
                                dic_motes[i].update({str(j):'-'})

                    log_parser.write('INFO: Creating data struct for all hops in the file: '+file+'\n')
                except:
                    print('Error, cannot create data struct in the file: '+ file)

                try:
                    
                    if args.nonstoring:
                        #Read M[id] - M[id2]:hops\n
                        hops_count_per_file(file_to_parse,dic_motes)

                        try: # Stats( Check if this simulation is:  OK / FAILED )
                            if get_state_of_sim(dic_motes,file_to_parse) :
                                sim_stats['sim_total_conv'] = str(int( sim_stats['sim_total_conv']) + 1)
                                sim_stats['sim_total_conv_seeds'].append(get_random_seed(file_to_parse))
                                is_a_no_conv_sim = False
                            else:
                                sim_stats['sim_total_no_conv'] = str(int( sim_stats['sim_total_no_conv']) + 1)
                                sim_stats['sim_total_no_conv_seeds'].append(get_random_seed(file_to_parse))
                                is_a_no_conv_sim = True
                            log_parser.write('INFO: Checking the state of the simulation in the file: '+ file +'\n')
                        except:
                            print('Error, cannot check the state of the simulation in the file: '+ file +'\n')
                        
                        #We are going to complete all posible paths (only non-storing mode)
                        if args.nonstoring:
                            
                            #If there aren't info about the hops to the sink mote, 
                            #We can try to recover this info from ipv6
                            sim_conv = complete_hops_allWall_ns(file_to_parse,dic_motes,dic_motes_ip, sim_stats, is_a_no_conv_sim)
                            
                            #if the simulation didnt converge, just log it
                            if not sim_conv:
                                log_parser.write('Error: Not converged \n')

                        log_parser.write('INFO: reading the hops per mote in the file: ' + file +'\n')
                       
                    elif args.storing:

                        try:
                            if get_state_of_sim(dic_motes,file_to_parse):
                                sim_conv = True  
                                dic_routing_tables = read_routes_per_motes(file_to_parse,num_motes,dic_motes_ip)
                                log_parser.write('INFO: Creating data struct for routing tables in the file: '+file+'\n')
                        except:
                            print('Error, cannot create data struct for routing tables in the file: '+file)

                        try:
                            if get_state_of_sim(dic_motes,file_to_parse):
                                sim_conv = True  
                                complete_hops_allWall_s(file_to_parse,dic_motes,dic_motes_ip, sim_stats,dic_routing_tables)
                                log_parser.write('INFO: reading the hops per mote in the file: ' + file +'\n')
                        except:
                            print('Error, cannot read the hops per mote (through routing table) in the file: '+ file)

                except:
                    print('Error, cannot read the hops per mote in the file: '+ file)

                try:
                    #Just write final result if the simulation converged
                    if sim_conv:
                        write_parsed_data(file_parsed,dic_motes,file,num_motes)
                        log_parser.write('INFO: writing in the file: '+ file+'\n')
                except:
                    print('Error, cannot write in the file: '+ file)
                
                #Close the files
                file_parsed.close()
                file_to_parse.close()

                #Manage if the simulation didnt converge
                if not sim_conv:
                    print('Machacando la simulacion no conv '+file+'\n')
                    manage_not_converged_sim(file,path,result_dir)
    
    #Add the stats in log file
    add_stats_to_results_file(sim_stats,path)
    log_parser.close()
    #Just for order the dir tree
    orderLogDir(path, sim_stats)




#Returns the seed of the simulation
def get_random_seed(file_to_parse):
    #   line == Random seed: 123486
    #   aux_line[1] ==  123486
    for line in file_to_parse:
        if line.count('Random seed: '):
            aux_line = line.split(':')
            file_to_parse.seek(0)
            return aux_line[1][1:len(aux_line[1]) -1 ]

    file_to_parse.seek()

#Return the number of motes in the simulation
def motes_count_per_file(file):
    aux_id = []

    # line == '31804806 ID:1 [INFO: Main      ] Node ID: 1'
    # aux_line[1] == '1 [INFO: Main      ] Node '
    # aux_line2[0]== '1 ' --> '12 '
    # aux_line2[0][0: len(aux_line2[0]) - 1] == 'ID'
    for line in file:
        if(line.count('Node ID: ')):                                            
            aux_line = line.split('ID: ')                                      
            aux_line2 = aux_line[1].split('[')
                                             
            if(not(int(aux_line2[0][0: len(aux_line2[0]) - 1]) in aux_id)):        
                aux_id.append(int(aux_line2[0][0: len(aux_line2[0]) - 1]))

    file.seek(0)  

    return len(aux_id)

#Returns a dic with key: node_id , value: IPV6(local) 
# (Ej: 2 nodes )--> {0 : 'Nothing here', 1:'fd00::212:7101:1:101',1:'fd00::212:7202:2:202'}
def read_ip_modes(file_to_parse):
    dic = {0 : 'Nothing here'}
    ipv6_link_local = ''
    ipv6_local = ''

    for line in file_to_parse:
    # '563087 ID:4 [INFO: Main      ] Tentative link-local IPv6 address: fe80::212:7404:4:404'                  
        if(line.count('Tentative link-local IPv6 address:')):

            aux_line = line.split('ID:')                               
            aux_line2 = aux_line[1].split('[')

            aux_str_ip = line.split('address:')

            #Casting link-local ipv6 to local ipv6 ->( fe80:: ) to ( fd00:: )
            ipv6_link_local = aux_str_ip[1][1:len(aux_str_ip[1]) -1]
            ipv6_local = get_ipv6_local(ipv6_link_local)

            dic[int(aux_line2[0][0: len(aux_line2[0]) - 1])] = ipv6_local

    file_to_parse.seek(0)
    return dic

#Parse from ipv6 link local to ipv6 local ( fe80:: ) to ( fd00:: ) 
def get_ipv6_local(ipv6_link_local):

    return ('fd00' + ipv6_link_local[4:])  

#Returns True or Flase in function of the state of the simulation (True : TEST OK | True : TEST FAILED )
def get_state_of_sim(dic_motes,file_to_parse):

    for line in file_to_parse:
        if line.count('TEST FAILED'):
            file_to_parse.seek(0)
            return False
        elif line.count('TEST OK'):
            file_to_parse.seek(0)
            return True

      

#Reads M[id]-M[id]:hops
def hops_count_per_file(file_to_parse,dic_motes):
    mote_a = 0
    mote_b = 0
    hops_a_to_b = 0

    for line in file_to_parse:
        if(line.count('M[')):
            data = line.split('M[') #Data[1] and data[2] 

            mote_a = int(data[1][0:len(data[1])-2])
            aux_data_split = data[2].split(':')
            
            mote_b = int(aux_data_split[0][0:len(aux_data_split[0])-1])
            hops_a_to_b = int(aux_data_split[1][0:len(aux_data_split[1])-1])
            
            dic_motes[mote_a][str(mote_b)]= str(hops_a_to_b)
            dic_motes[mote_b][str(mote_a)]= str(hops_a_to_b)

    file_to_parse.seek(0)

#Only in non-storing mode
def complete_hops_allWall_ns(file_to_parse,dic_motes, dic_ip, sim_stats, is_a_no_conv_sim):
    bool_sim_conv = True
    have_used_use_ip_hops_check = False

    #Initial check for hops to sink
    for i in range(2, motes_count_per_file(file_to_parse) + 1):
        if dic_motes[1][str(i)] == '-':
            have_used_use_ip_hops_check = True
            bool_sim_conv = use_ip_hops_check(file_to_parse,dic_motes, dic_ip, str(i))
            if  not bool_sim_conv:
                sim_stats['sim_total_no_recovered_seeds'].append(get_random_seed(file_to_parse))
                sim_stats['sim_total_no_recovered'] = str(int( sim_stats['sim_total_no_recovered']) + 1)
                return False

    if (have_used_use_ip_hops_check and is_a_no_conv_sim) or (( not have_used_use_ip_hops_check) and is_a_no_conv_sim):        
        sim_stats['sim_total_recovered_seeds'].append(get_random_seed(file_to_parse))
        sim_stats['sim_total_recovered'] = str(int( sim_stats['sim_total_recovered']) + 1)           
    

    #Complete all posible paths 
    for i in range(2, motes_count_per_file(file_to_parse) + 1):
        for j in range(2, motes_count_per_file(file_to_parse) +1):
            if i != j:
                dic_motes[i][str(j)] = str(int(dic_motes[1][str(i)]) + int(dic_motes[1][str(j)]))

    return True

#Generate parser stats
def add_stats_to_results_file(sim_stats,path):

    temp_file = open(path+'/'+ 'ParserResults.txt', 'w')
    temp_file.write('\nParser statistics:\n\n')
    temp_file.write('1.\tTotal simulations: '+ sim_stats['sim_total']+'\n')
    temp_file.write('2.\tTotal simulations conv: '+ sim_stats['sim_total_conv']+' ( {:.3f}% )\n'.format(100*float(int(sim_stats['sim_total_conv'])/int(sim_stats['sim_total']))))
    temp_file.write('3.\tTotal simulations no conv: '+sim_stats['sim_total_no_conv']+' ( {:.3f}% )\n'.format(100*float(int(sim_stats['sim_total_no_conv'])/int(sim_stats['sim_total']))))
    if int(sim_stats['sim_total_no_conv']) != 0 :
        temp_file.write('4.\tTotal simulations recovered: '+sim_stats['sim_total_recovered']+' ( {:.3f}% )\n'.format(100*float(int(sim_stats['sim_total_recovered'])/int(sim_stats['sim_total_no_conv']))))
    else:
        temp_file.write('4.\tTotal simulations recovered: '+sim_stats['sim_total_recovered']+' ( {:.3f}% )\n'.format(100*float(int(sim_stats['sim_total_recovered'])/int(sim_stats['sim_total']))))
    temp_file.write('5.\tTotal simulations no recovered: '+sim_stats['sim_total_no_recovered']+' ( {:.3f}% )\n'.format(100*float(int(sim_stats['sim_total_no_recovered'])/int(sim_stats['sim_total'])))) 
    temp_file.write('\n\n-- Parser summary by seeds --\n\n')
    temp_file.write('|     Total simulations     |\n')
    for i in range(0, len(sim_stats['sim_total_seeds'])):
        temp_file.write('\t\t\t\t\t' + sim_stats['sim_total_seeds'][i] + '\t|\n')
    
    temp_file.write('\n\n|  Total simulations Conv   |\n')
    for i in range(0, len(sim_stats['sim_total_conv_seeds'])):
        temp_file.write('\t\t\t\t\t' + sim_stats['sim_total_conv_seeds'][i] + '\t|\n')
    
    temp_file.write('\n\n| Total simulations No Conv |\n')
    for i in range(0, len(sim_stats['sim_total_no_conv_seeds'])):
        temp_file.write('\t\t\t\t\t' + sim_stats['sim_total_no_conv_seeds'][i] + '\t|\n')

    temp_file.write('\n\n|         Recovered         |\n')
    for i in range(0, len(sim_stats['sim_total_recovered_seeds'])):
        temp_file.write('\t\t\t\t\t' + sim_stats['sim_total_recovered_seeds'][i] + '\t|\n')
    
    temp_file.write('\n\n|        No Recovered       |\n')
    for i in range(0, len(sim_stats['sim_total_no_recovered_seeds'])):
        temp_file.write('\t\t\t\t\t' + sim_stats['sim_total_no_recovered_seeds'][i] + '\t|\n')

#Reads Ip: ip hops\n
def use_ip_hops_check(file_to_parse,dic_motes, dic_ip, key):
    bool_sim_conv = False

    for line in file_to_parse:
        if line.count('Ip: '+dic_ip[int(key)]):
            aux_line = line.split(dic_ip[int(key)])
            aux_line2 = aux_line[1].split(': ')

            dic_motes[1][key] = dic_motes[int(key)]['1'] = aux_line2[1][0: len(aux_line2[1]) - 1]
            bool_sim_conv = True

    file_to_parse.seek(0)

    return bool_sim_conv

def get_node_id_from_ipv6(dic_motes_ip, ipv6): # ( fd00:: ) ipv6 is required

    for node_id,ipv6_dic in dic_motes_ip.items():
        if ipv6_dic == ipv6:
            return str(node_id)

    return 'Not found'

def read_routes_per_motes(file_to_parse, num_motes,dic_motes_ip):
    dic = {0 : 'Nothing here'}
    
    for i in range(1,num_motes+1):
        dic[i]  = {}
        for j in range(1,(num_motes+1)):
            if j != i:
                dic[i].update({str(j):'-'})

    #Read routing table from the file
    for line in file_to_parse:
        if line.count('through'):
            aux_line = line.split('through ')           # aux_line = ['00:30.718	ID:1	M[1]: fd00::212:7402:2:202 ','fe80::212:7405:5:505\n']

            #For via route
            via = get_node_id_from_ipv6(dic_motes_ip, get_ipv6_local( aux_line[1][0:len(aux_line[1]) - 1] ))

            node_id_and_route = aux_line[0].split('M[')    # node_id_and_route = ['00:30.718	ID:1	','1]: fd00::212:7402:2:202 ']
            aux_line_2 = node_id_and_route[1].split(']:')     # aux_line_2 = ['1',' fd00::212:7402:2:202 ']

            #For node id
            node_id = aux_line_2[0]

            #For route
            route = get_node_id_from_ipv6(dic_motes_ip,aux_line_2[1][1:len(aux_line_2[1]) - 1])

            #Save into dic
            dic[int(node_id)][route] = via

            if route == via:
                dic[int(route)][node_id] = 'father'

    #Complete our routing table
    for mote in range(1,num_motes +1):
        for dest_mote in range(1,num_motes+1):
            if mote != dest_mote: 
                if dic[mote][str(dest_mote)] == '-':
                    for j in range(1,num_motes + 1):
                        if mote != j:
                            if dic[mote][str(j)] == 'father':
                                dic[mote][str(dest_mote)] = str(j)
                                
    file_to_parse.seek(0)
    return dic


def complete_hops_allWall_s(file_to_parse,dic_motes,dic_motes_ip, sim_stats,dic_routing_tables):

    for mote in range(1, motes_count_per_file(file_to_parse) +1):
        for dest_mote in range(1, motes_count_per_file(file_to_parse) +1 ):
            if mote != dest_mote:
                if dic_routing_tables[mote][str(dest_mote)] == str(dest_mote):
                    dic_motes[mote][str(dest_mote)] = '1'
                    

                elif dic_routing_tables[mote][str(dest_mote)] == 'father':
                    dic_motes[mote][str(dest_mote)] = '1'
                    

                elif dic_routing_tables[mote][str(dest_mote)] != str(dest_mote):
                    dic_motes[mote][str(dest_mote)] = '1'
                    next_hop = dic_routing_tables[mote][str(dest_mote)]
         
                    while next_hop != str(dest_mote):
                        dic_motes[mote][str(dest_mote)] = str(int(dic_motes[mote][str(dest_mote)]) + 1)
                        next_hop = dic_routing_tables[int(next_hop)][str(dest_mote)]

                        if dest_mote == 1 and next_hop == 'father':
                            break



#Write the parsed data
def write_parsed_data(file_parsed,dic_motes,file,num_motes):
    file_parsed.write('File generated by hopscount_parser.py \n')
    file_parsed.write('Original file name: '+ file + '\n')
    file_parsed.write('Number of motes: '+str(num_motes) +'\n')
    avg=0.0
    aux_avg=0.0
    num_motes_aux=0

    dic_avg_per_mote = {'0':'-'}
    for i in range(1,num_motes+1):
        dic_avg_per_mote.update({str(i):'0'})

    

    for i in range(1,num_motes+1):
        file_parsed.write('\n\nMote['+str(i)+'] | Dest_Mote_ID | Hops\n')
        
        for key in sorted(dic_motes[i]):
            file_parsed.write('\t\t| \t\t\t'+str(key)+' | \t'+ str(dic_motes[i][key])+'\n')
            if(str(dic_motes[i][key]) != '-'):
                avg += float(dic_motes[i][key])
                num_motes_aux+=1
        
        avg /= float(num_motes_aux)
        file_parsed.write('Hops Avg to all motes: '+ str(avg) +'\n')
        dic_avg_per_mote[str(i)] = str(avg)
        avg=0.0
        num_motes_aux=0
    
    for i in range(1,num_motes+1):
        aux_avg+=float(dic_avg_per_mote[str(i)])

    aux_avg /=  float(num_motes)
    file_parsed.write('\n|Total Hops avg all with all: '+str(aux_avg) +'\n') 

#Just for order the dirs and parsed data 
def orderLogDir(path, sim_stats):
    cooja_data = 'Cooja_logs'
    raw_data = 'Raw_data'

    #Create dir structe
    os.mkdir(path+'/'+ cooja_data + '/')
    os.mkdir(path +'/'+raw_data+'/')
    os.mkdir(path +'/'+raw_data+'/converged')
    os.mkdir(path +'/'+raw_data+'/recovered')
    os.mkdir(path +'/'+raw_data+'/no_converged')

    #Move files
    os.system('mv ' + path + '/*.coojalog '+ path + '/' + cooja_data + '/')
    for recovered_files in sim_stats['sim_total_recovered_seeds']:
        os.system('mv ' + path + '/*'+recovered_files+'.scriptlog '+ path + '/' + raw_data + '/recovered')
    if int(sim_stats['sim_total_no_recovered']) != 0:
        os.system('mv ' + path + '/no_conv_*.scriptlog '+ path + '/' + raw_data + '/no_converged')
    

    os.system('mv ' + path + '/*.scriptlog '+ path + '/' + raw_data + '/converged')
    

#Change the name of no conv/recovered files
def manage_not_converged_sim(file,path,result_dir):

    os.rename(path +'/'+ file, path +'/no_conv_' +file)


#Main
if __name__ == '__main__':

    #Check args
    if(len(sys.argv) != 3):
        print('Error: usage: ' + sys.argv[0] + ' -[ns | s]  <dir_log>\n')
        sys.exit(0)

    # sys.argv[2] == 'log05' for example
    base_name = sys.argv[2]

    #Main function of the parser
    hopscount_parser(base_name)

    print('\nParser status: success')

