#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
纽约市平面地图生成器
New York City Map Generator
"""

import folium
import pandas as pd

def create_nyc_map():
    """创建纽约市的交互式地图"""
    
    # 纽约市中心坐标 (时代广场附近)
    nyc_center = [40.7589, -73.9851]
    
    # 创建基础地图
    nyc_map = folium.Map(
        location=nyc_center,
        zoom_start=11,
        tiles='OpenStreetMap'
    )
    
    # 纽约市主要地标数据
    landmarks = [
        # 曼哈顿主要地标
        {"name": "时代广场 Times Square", "lat": 40.7589, "lng": -73.9851, "color": "red"},
        {"name": "中央公园 Central Park", "lat": 40.7829, "lng": -73.9654, "color": "green"},
        {"name": "帝国大厦 Empire State Building", "lat": 40.7484, "lng": -73.9857, "color": "blue"},
        {"name": "自由女神像 Statue of Liberty", "lat": 40.6892, "lng": -74.0445, "color": "orange"},
        {"name": "布鲁克林大桥 Brooklyn Bridge", "lat": 40.7061, "lng": -73.9969, "color": "purple"},
        {"name": "华尔街 Wall Street", "lat": 40.7074, "lng": -74.0113, "color": "darkgreen"},
        {"name": "联合国总部 UN Headquarters", "lat": 40.7489, "lng": -73.9680, "color": "lightblue"},
        {"name": "洋基体育场 Yankee Stadium", "lat": 40.8296, "lng": -73.9262, "color": "darkblue"},
        {"name": "大都会艺术博物馆 Metropolitan Museum", "lat": 40.7794, "lng": -73.9632, "color": "pink"},
        {"name": "9/11纪念馆 9/11 Memorial", "lat": 40.7115, "lng": -74.0134, "color": "black"},
        
        # 各区中心
        {"name": "曼哈顿 Manhattan", "lat": 40.7831, "lng": -73.9712, "color": "red"},
        {"name": "布鲁克林 Brooklyn", "lat": 40.6782, "lng": -73.9442, "color": "blue"},
        {"name": "皇后区 Queens", "lat": 40.7282, "lng": -73.7949, "color": "green"},
        {"name": "布朗克斯 Bronx", "lat": 40.8448, "lng": -73.8648, "color": "orange"},
        {"name": "史泰登岛 Staten Island", "lat": 40.5795, "lng": -74.1502, "color": "purple"},
    ]
    
    # 在地图上添加地标
    for landmark in landmarks:
        folium.Marker(
            location=[landmark["lat"], landmark["lng"]],
            popup=folium.Popup(landmark["name"], parse_html=True),
            tooltip=landmark["name"],
            icon=folium.Icon(color=landmark["color"], icon='info-sign')
        ).add_to(nyc_map)
    
    # 添加纽约市五个区的边界（简化版）
    boroughs_boundaries = [
        # 曼哈顿边界（简化）
        {
            "name": "曼哈顿 Manhattan",
            "coordinates": [
                [40.8007, -73.9512], [40.7831, -73.9712], [40.7489, -73.9680],
                [40.7074, -74.0113], [40.7000, -74.0200], [40.7150, -74.0134],
                [40.7589, -73.9851], [40.8007, -73.9512]
            ],
            "color": "red"
        },
        # 布鲁克林边界（简化）
        {
            "name": "布鲁克林 Brooklyn", 
            "coordinates": [
                [40.7361, -73.9906], [40.6782, -73.9442], [40.5795, -74.0000],
                [40.6000, -74.0500], [40.7000, -74.0200], [40.7361, -73.9906]
            ],
            "color": "blue"
        }
    ]
    
    # 添加区域边界
    for borough in boroughs_boundaries:
        folium.Polygon(
            locations=borough["coordinates"],
            popup=borough["name"],
            color=borough["color"],
            weight=3,
            fill=True,
            fillColor=borough["color"],
            fillOpacity=0.1
        ).add_to(nyc_map)
    
    # 添加不同的地图图层
    folium.TileLayer(
        tiles='https://stamen-tiles-{s}.a.ssl.fastly.net/terrain/{z}/{x}/{y}{r}.png',
        attr='Map tiles by <a href="http://stamen.com">Stamen Design</a>, <a href="http://creativecommons.org/licenses/by/3.0">CC BY 3.0</a> &mdash; Map data &copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors',
        name='Stamen Terrain',
        subdomains='abcd'
    ).add_to(nyc_map)
    
    folium.TileLayer('CartoDB positron').add_to(nyc_map)
    
    # 添加图层控制
    folium.LayerControl().add_to(nyc_map)
    
    # 添加地图标题
    title_html = '''
    <h3 align="center" style="font-size:20px"><b>纽约市地图 - New York City Map</b></h3>
    '''
    nyc_map.get_root().html.add_child(folium.Element(title_html))
    
    return nyc_map

def save_static_map():
    """创建并保存静态地图图片"""
    import matplotlib.pyplot as plt
    import matplotlib.patches as patches
    
    # 创建静态地图
    fig, ax = plt.subplots(figsize=(12, 10))
    
    # 设置地图范围（纽约市大致范围）
    ax.set_xlim(-74.3, -73.7)
    ax.set_ylim(40.5, 40.9)
    
    # 添加主要地标点
    landmarks_simple = [
        {"name": "时代广场", "lat": 40.7589, "lng": -73.9851},
        {"name": "中央公园", "lat": 40.7829, "lng": -73.9654},
        {"name": "帝国大厦", "lat": 40.7484, "lng": -73.9857},
        {"name": "布鲁克林大桥", "lat": 40.7061, "lng": -73.9969},
        {"name": "自由女神像", "lat": 40.6892, "lng": -74.0445},
    ]
    
    # 绘制地标点
    for landmark in landmarks_simple:
        ax.plot(landmark["lng"], landmark["lat"], 'ro', markersize=8)
        ax.annotate(landmark["name"], 
                   (landmark["lng"], landmark["lat"]),
                   xytext=(5, 5), textcoords='offset points',
                   fontsize=9, ha='left',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor='yellow', alpha=0.7))
    
    # 添加简化的纽约市轮廓
    manhattan_x = [-74.02, -73.93, -73.93, -74.02, -74.02]
    manhattan_y = [40.70, 40.70, 40.80, 40.80, 40.70]
    ax.plot(manhattan_x, manhattan_y, 'b-', linewidth=2, label='曼哈顿')
    
    brooklyn_x = [-74.05, -73.88, -73.88, -74.05, -74.05]
    brooklyn_y = [40.58, 40.58, 40.72, 40.72, 40.58]
    ax.plot(brooklyn_x, brooklyn_y, 'g-', linewidth=2, label='布鲁克林')
    
    # 设置标题和标签
    ax.set_title('纽约市平面地图 - New York City Map', fontsize=16, fontweight='bold')
    ax.set_xlabel('经度 Longitude')
    ax.set_ylabel('纬度 Latitude')
    ax.grid(True, alpha=0.3)
    ax.legend()
    
    # 保存静态地图
    plt.tight_layout()
    plt.savefig('nyc_static_map.png', dpi=300, bbox_inches='tight')
    plt.savefig('nyc_static_map.pdf', bbox_inches='tight')
    print("静态地图已保存为 nyc_static_map.png 和 nyc_static_map.pdf")

def main():
    """主函数"""
    print("正在生成纽约市地图...")
    
    # 创建交互式地图
    nyc_map = create_nyc_map()
    
    # 保存HTML地图
    nyc_map.save('nyc_interactive_map.html')
    print("交互式地图已保存为 nyc_interactive_map.html")
    
    # 创建并保存静态地图
    save_static_map()
    
    print("\n地图生成完成！")
    print("文件列表：")
    print("- nyc_interactive_map.html (可在浏览器中打开的交互式地图)")
    print("- nyc_static_map.png (静态地图图片)")
    print("- nyc_static_map.pdf (静态地图PDF)")

if __name__ == "__main__":
    main()