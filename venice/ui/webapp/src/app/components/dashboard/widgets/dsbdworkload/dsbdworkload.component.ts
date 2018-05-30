import { Component, OnChanges, OnDestroy, OnInit } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { WorkloadService } from '@app/services/workload.service';
import { Subscription } from 'rxjs/Subscription';

@Component({
  selector: 'app-dsbdworkloadwidget',
  templateUrl: './dsbdworkload.component.html',
  styleUrls: ['./dsbdworkload.component.scss']
})
export class DsbdworkloadComponent implements OnInit, OnDestroy, OnChanges {
  _background_img: any;
  id: string;
  title = 'Workloads';
  content = 'Workloads';
  last_update: string = 'Last Updated: ' + Utility.getPastDate(3).toLocaleDateString();
  background_img: any = {
    url: 'assets/images/dashboard/ico-workload.svg'
  };
  menuItems: any[] = [
    { text: 'Trend line' },
    { text: 'Trouble shooting' },
    { text: 'Search Workload' }
  ];
  icon: Icon = {
    margin: {
      top: '8px',
      left: '10px'
    },
    svgIcon: 'workloads-unprotected'
  };
  subscription: Subscription;
  veniceEndpointCount = 0;
  veniceNewEndpointCount = 0;
  data: any;
  options: any;

  constructor(private _workloadService: WorkloadService) {
  }

  ngOnInit() {
    this.id = 'workload-' + Utility.s4();
    this.getWidgetData();
    this._background_img = this.setBackgroundImg();
  }

  menuselect(obj) {
    console.log('workload menuselect()', obj);
  }

  getWidgetData() {
    this.subscription = this._workloadService.getDsbdWidgetData().subscribe(data => {
      this.formatData(data);
    });
  }

  formatData(data) {
    console.log('format', data);
    this.veniceEndpointCount = data.veniceEndpointCount;
    this.veniceNewEndpointCount = data.veniceNewEndpointCount;
    const endpointList = data.endpointList;
    const endpointPercent = data.endpointPercent;
    const endpointPercentCompliment = data.endpointPercentCompliment;
    this.data = {
      labels: ['Week1', 'Week2', 'Weak3'],
      datasets: [
        {
          type: 'line',
          label: 'Endpoint-(1000)',
          borderColor: '#97b8df',
          borderWidth: 2,
          fill: false,
          data: endpointList
        },
        {
          type: 'bar',
          label: 'Increase %',
          backgroundColor: '#94d4c6',
          data: endpointPercent
        }, {
          type: 'bar',
          label: 'Capacity %',
          backgroundColor: '#eee',
          data: endpointPercentCompliment,
        }]
    };
    this.options = {
      title: {
        display: false,
        text: 'Workloads'
      },
      tooltips: {
        mode: 'index',
        intersect: false
      },
      legend: {
        display: false
      },

      responsive: true,
      scales: {
        xAxes: [{
          gridLines: {
            display: false,
            drawBorder: false
          },
          stacked: true,
        }],
        yAxes: [{
          display: false,
          stacked: true,
          gridLines: {
            display: false,
            drawBorder: false
          }
        }]
      }, plugins: {
        datalabels: {
          color: '#61b3a0',
          align: 'after',
          display: function(context) {
            return context.datasetIndex === 2;
          },
          formatter: function(value, context) {
            return Math.round(100 - value) + '% UP';
          },
          font: {
            weight: 'bold',
            family: 'Fira Sans Condensed'
          },
          offset: 100

        }
      }
    };
  }

  setBackgroundImg() {
    const styles = {
      'background-image': 'url(' + this.background_img.url + ')',
    };
    return styles;
  }

  ngOnDestroy() {
    if (this.subscription != null) {
      this.subscription.unsubscribe();
    }
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
