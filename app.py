# pip install streamlit-echarts


import streamlit as st
import pandas as pd
import requests
from datetime import datetime, timezone
from streamlit_echarts import st_echarts
import streamlit.components.v1 as components

st.set_page_config(layout="wide")

def create_gaung(value,name,color,unit):
    option = {
        "tooltip": {
            "formatter": '{a} <br/>{b} : {c}%'
        },
        "series": [{
            "name": '进度',
            "type": 'gauge',
            "startAngle": 180,
            "endAngle": 0,
            "progress": {
                "show": "true"
            },
            "radius":'100%', 

            "itemStyle": {
                "color": color,
                "shadowColor": 'rgba(0,138,255,0.45)',
                "shadowBlur": 10,
                "shadowOffsetX": 2,
                "shadowOffsetY": 2,
                "radius": '55%',
            },
            "progress": {
                "show": "true",
                "roundCap": "true",
                "width": 15
            },
            "pointer": {
                "length": '60%',
                "width": 8,
                "offsetCenter": [0, '5%']
            },
            "detail": {
                "valueAnimation": "true",
                "formatter": f'{value}{unit}',
                "backgroundColor": 'white', #'#58D9F9',
                "borderColor": color, #'#999',
                "borderWidth": 4,
                "width": '60%',
                "lineHeight": 20,
                "height": 20,
                "borderRadius": 188,
                "offsetCenter": [0, '40%'],
                "valueAnimation": "true",
            },
            "data": [{
                "value": value,
                "name": name
            }]
        }]
    };

    return option

tab1, tab2, tab3 = st.tabs(["Home", "History", "Setting"])
user_id = None
with tab1:
    st.header('Welcome to HT-Army App!')

    components.iframe("https://docs.google.com/presentation/d/e/2PACX-1vTCwBK9YbE9uanDp3wBEUrGmgT75ymZlCP2AUuBd8xjADCbBUApSbNGSnRHOPixNGhJ34dgWvfR8g3q/embed?start=false&loop=false&delayms=3000", height=480)

    user_input = st.text_input('#### Enter ID:')
    if user_input:
        with st.spinner('Wait for it...'):
            url = f"https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?id={user_input}&action=getConfig"
            response = requests.get(url)

        if response.json():
            config_data = response.json()
            # st.write(response.json())

            user_id = response.json()['id']

            st.write(f"#### Welcome! {response.json()['unit']}")
            # st.write("#### History Data")

        else:
            st.write('ID not exist')

    isdata = False
    if user_id:
        with st.spinner('Wait for it...'):
            url = f"https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?action=getDatas&id={user_id}"
            response = requests.get(url)

        if response.status_code == 200:
            try:
                data = response.json()
                isdata = True
            except:
                isdata = False



        if isdata:

            df_history = pd.DataFrame(data)
            df_history['temp'] = df_history['temp'].astype(float)
            df_history['humid'] = df_history['humid'].astype(float)
            df_history['hic'] = df_history['hic'].astype(float)

            # df_history['co2'] = df_history['co2'].astype(float)
            df_history['co2'] = df_history['co2'].replace('', float('nan'))
            df_history['co2'] = df_history['co2'].astype(float)


            # df_history['datetime'] = [datetime.datetime.fromtimestamp(x) for x in df_history['date']]
            df_history['datetime'] = [datetime.fromtimestamp(x, timezone.utc) for x in df_history['date']]
           

            last_date = df_history[df_history['date'] == df_history['date'].max()]

            st.write(f"#### Lastseen {last_date['datetime'].iloc[0]}")

            col1, col2 = st.columns([1,1])
            with col1:
                value,name,color = last_date['co2'].iloc[0],'CO2','#097969'
                option = create_gaung(int(value),name,color,'')
                st_echarts(options=option, key="1")

                value,name,color = last_date['hic'].iloc[0],'Heat Index','red'
                option = create_gaung(round(value, 1),name,color,'°C')
                st_echarts(options=option, key="2")


            with col2:
                
                value,name,color = last_date['temp'].iloc[0],'Temperature','red'
                option = create_gaung(round(value, 1),name,color,'°C')
                st_echarts(options=option, key="3")

                value,name,color = last_date['humid'].iloc[0],'Humidity','#58D9F9'
                option = create_gaung(round(value, 1),name,color,'%')
                st_echarts(options=option, key="4")            

with tab2:
    if user_id:
        st.write('### Historys')

        if isdata:

            df_history = df_history.drop(columns=['date'])

            df_history['co2'] = df_history['co2'].astype(int)

            df_history.rename(columns={'datetime': 'Timestamp'}, inplace=True)
            df_history.rename(columns={'id': 'Device ID'}, inplace=True)
            df_history.rename(columns={'temp': 'Temperature(°C)'}, inplace=True)
            df_history.rename(columns={'humid': 'Humidity(%)'}, inplace=True)
            df_history.rename(columns={'hic': 'Heat index(°C)'}, inplace=True)
            df_history.rename(columns={'co2': 'CO2'}, inplace=True)
            
            st.line_chart(
            # df_history, x="datetime", y=["temp", "humid"], color=["#FF0000", "#0000FF"]  # Optional
            df_history, x="Timestamp", y=['CO2','Heat index(°C)',"Temperature(°C)", "Humidity(%)"]  # Optional
            )

            df_history.set_index('Timestamp', inplace=True)
            st.write(df_history)

            # st.table(df_history)

        
        
with tab3:
    st.write("### Setting")
    if user_id:
        for x in ['adj_temp','adj_humid','adj_co2']:
            if config_data[x] == "":
                config_data[x] = 0.0

            config_data[x] = float(config_data[x])
            


        new_config_data = {}
        new_config_data['id'] = config_data['id']
        new_config_data['unit'] = st.text_input('#####  Unit:',config_data['unit'])
        
        new_config_data['adj_temp'] = st.number_input('Adust temp error', value=config_data['adj_temp'])
        new_config_data['adj_humid'] = st.number_input('Adust humid error',value=config_data['adj_humid'])
        new_config_data['adj_co2'] = st.number_input('Adust PM2.5 error', value=config_data['adj_co2'])
        new_config_data['line1'] = st.text_input('#####  line1:',config_data['line1'])
        new_config_data['line2'] = st.text_input('#####  line2:',config_data['line2'])
        new_config_data['line3'] = st.text_input('#####  line3:',config_data['line3'])

        # st.write(new_config_data)

        if st.button('Update'):

            # post Api config
            url = "https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?action=addConfig"

            response = requests.post(url, json=new_config_data)

            if response.status_code == 200:
                st.write(response.text)
            else:
                st.write("Failed:", response.status_code, response.text)



        # "https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?action=addConfig"



#         {
# "id":"S005"
# "unit":"u5"
# "adj_temp":2.55
# "adj_humid":4.66
# "adj_co2":""
# "line1":"i4dIQOPNh6slv6gG9h5SoQdiq5SD2btX35Vg5IDl0Qm"
# "line2":"vsdvsd6"
# "line3":"vsdvsd7"
# }


    # else:
    #     st.write('ID not exist')





    
    
# user_input = st.text_input('#### Enter ID:')




# # if user_input and user_input in list(df['id']):
# if 

#     df_id = df[df['id']==user_input]
#     st.write('#### Unit:', df_id['unit'].iloc[0])

#     with st.spinner('Wait for it...'):
    
#         url = f"https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?id={user_input}"
#         response = requests.get(url)

#     if response.status_code == 200:
#         try:
#             data = response.json()
#             isdata = True
#         except:
#             isdata = False


#         if isdata:

#             df_history = pd.DataFrame(data)
#             df_history['temp'] = df_history['temp'].astype(float)
#             df_history['humid'] = df_history['humid'].astype(float)
#             df_history['hic'] = df_history['hic'].astype(float)

#             # df_history['co2'] = df_history['co2'].astype(float)
#             df_history['co2'] = df_history['co2'].replace('', float('nan'))
#             df_history['co2'] = df_history['co2'].astype(float)


#             df_history['datetime'] = [datetime.datetime.fromtimestamp(x) for x in df_history['date']]
#             last_date = df_history[df_history['date'] == df_history['date'].max()]

#             st.write(f"#### Lastseen {last_date['datetime'].iloc[0]}")

#             col1, col2, col3=st.columns([1,1,1])
#             with col1:
#                 value,name,color = last_date['hic'].iloc[0],'Heat Index',last_date['flag'].iloc[0]
#                 option = create_gaung(value,name,color)
#                 st_echarts(options=option, key="1")

#             with col2:
                
#                 value,name,color = last_date['temp'].iloc[0],'Temperature',last_date['flag'].iloc[0]
#                 option = create_gaung(value,name,color)
#                 st_echarts(options=option, key="2")

#             with col3:
                
#                 value,name,color = last_date['humid'].iloc[0],'Temperature','#58D9F9'
#                 option = create_gaung(value,name,color)
#                 st_echarts(options=option, key="3")

#             st.write('### Historys')

#             df_history = df_history.drop(columns=['date'])
            
#             st.line_chart(
#             df_history, x="datetime", y=["temp", "humid"], color=["#FF0000", "#0000FF"]  # Optional
#             )

#             df_history.set_index('datetime', inplace=True)
#             st.write(df_history)

#         else:

#             st.write('No history data')

#     else:
#         st.write(f"Failed to retrieve data: {response.status_code}")

    
#     st.markdown("[Setting](https://ht-army2024-f4e56.web.app/)")
        

# else:
#     st.write('user ID not exist!')
        


