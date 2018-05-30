import { Component, OnChanges, OnDestroy, OnInit } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';

@Component({
  selector: 'app-dsbdnapleswidget',
  templateUrl: './naples.component.html',
  styleUrls: ['./naples.component.scss']
})
export class NaplesComponent implements OnInit, OnDestroy, OnChanges {
  _background_img_left: any;
  _background_img_right: any;
  id: string;

  title = 'Naples';
  content = 'Naples';
  last_update: string = 'Last Updated: ' + Utility.getPastDate(3).toLocaleDateString();
  background_img_left: any = {
    url: 'assets/images/dashboard/ico-naples-yellow.svg'
  };
  background_img_right: any = {
    url: 'assets/images/dashboard/ico-magnifying-glass-color.svg'
  };
  menuItems: any[] = [
    { text: 'Trend line' },
    { text: 'Trouble shooting' },
    { text: 'Search Workload' }
  ];

  venice_naples_count: number;
  venice_naples_discovered_count: number = Utility.getRandomInt(10, 30);
  icon: Icon = {
    margin: {
      top: '8px',
      left: '10px'
    },
    svgIcon: 'workloads-unprotected'
  };
  data: any;
  options: any;

  constructor() {
  }

  ngOnInit() {
    if (!this.id) {
      this.id = 'naples-' + Utility.s4();
    }
    this.generateData();

    this._background_img_left = this.setLeftBackgroundImg();
    this._background_img_right = this.setRightBackgroundImg();
  }

  private _getNaplesNumbers(): any {
    const pct = Utility.getRandomInt(60, 97);
    const total = Utility.getRandomInt(1000, 3000);
    const goodNum = Math.floor(total * pct / 100);
    const badNum = total - goodNum;
    const obj = {};
    obj['total'] = total;
    obj['goodNum'] = goodNum;
    obj['badNum'] = badNum;
    obj['goodLabel'] = 'Good';
    obj['badLabel'] = 'Bad';
    obj['percent'] = pct + '%';
    return obj;
  }

  menuselect(item) {
    console.log('naples menuselect()', item);
  }

  isToShowNapleChartTooltip(tooltipItem, data): boolean {
    if (tooltipItem.datasetIndex === 0 && tooltipItem.index === 1) {
      return false;
    }
    if (tooltipItem.datasetIndex === 1 && tooltipItem.index === 1) {
      return false;
    }
    return true;
  }

  generateData() {
    const obj = this._getNaplesNumbers();
    const badLabel = obj['badLabel'];
    const goodLabel = obj['goodLabel'];
    const badNum: Number = obj['badNum'];
    const goodNum: Number = obj['goodNum'];
    const numPercent = obj['percent'];

    this.venice_naples_count = obj['total'];
    this.data = {
      labels: [goodLabel, badLabel],
      datasets: [
        {
          data: [goodNum, badNum],
          backgroundColor: [
            '#88b358',
            '#eeeeee'
          ]
        },
        {
          data: [badNum, goodNum],
          backgroundColor: [
            '#e57553',
            '#eeeeee'
          ]
        }
      ]
    },
      this.options = {
        circumference: 1.8 * Math.PI,
        rotation: -1.4 * Math.PI,
        cutoutPercentage: 70,
        title: {
          display: false
        },
        legend: {
          display: false
        },
        plugins: {
          datalabels: {
            display: false
          }
        },
        tooltips: {
          enabled: true,
          filter: (tooltipItem, data) => {
            return this.isToShowNapleChartTooltip(tooltipItem, data);
          },
          callbacks: {

            title: function(tooltipItem, data) {
              return (tooltipItem.length > 0) ? data['labels'][tooltipItem[0]['datasetIndex']] : null;
            },
            label: function(tooltipItem, data) {
              return (tooltipItem) ? data['datasets'][tooltipItem.index].data[tooltipItem.datasetIndex] : null;
            },


          }
        },
        animation: {
          onComplete: function() {
            //
            // see. dashboard.component.html <ng-template #dashboardPolicyHealth . There is a <canvas id="policy_health_text"
            // we use the chart.js onComplete() to draw labels to the center of donut chart
            //
            const mainLabel = numPercent;
            const $ = Utility.getJQuery();
            const element = $('#naples_text').get(0);
            const tag = (goodNum > badNum) ? 'Good' : 'Bad';
            if (element) {
              const textCtx = element.getContext('2d');
              textCtx.textAlign = 'center';
              textCtx.textBaseline = 'middle';
              textCtx.font = '36px Fira San';
              textCtx.fillStyle = '#88b358';
              textCtx.fillText(mainLabel, 185, 80);
              textCtx.font = '10px Fira San';
              textCtx.fillStyle = '#676763';
              textCtx.fillText(tag, 185, 100);
              textCtx.font = '10px Fira San';
              textCtx.fillStyle = '#e57553';
              textCtx.fillText(badNum, 185, 130);
              textCtx.font = '12px Fira San';
              textCtx.fillStyle = '#88b358';
              textCtx.fillText(goodNum, 185, 145);
            }
          }
        }
      };
  }

  setLeftBackgroundImg() {
    const styles = {
      'background-image': 'url(' + this.background_img_left.url + ')',
    };
    return styles;
  }

  setRightBackgroundImg() {
    const styles = {
      'background-image': 'url(' + this.background_img_right.url + ')',
    };
    return styles;
  }

  ngOnDestroy() {
  }

  ngOnChanges() {
  }

  itemClick($event) {
    const obj = {
      id: this.id,
      event: $event
    };
    console.log(this.id, obj);
  }

}
