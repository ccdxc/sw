import { Component, OnChanges, OnDestroy, OnInit } from '@angular/core';
import { MockDataUtil } from '@app/common/MockDataUtil';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';

@Component({
  selector: 'app-dsbdpolicyhealthwidget',
  templateUrl: './policyhealth.component.html',
  styleUrls: ['./policyhealth.component.scss']
})
export class PolicyhealthComponent implements OnInit, OnDestroy, OnChanges {
  _background_img: any;
  id: string;

  title = 'Policies Health';
  content = 'Policies Health';
  last_update: string = 'Last Updated: ' + Utility.getPastDate(3).toLocaleDateString();
  background_img: any = {
    url: 'policy-health'
  };

  menuItems: any[] = [
    { text: 'Trend line' },
    { text: 'Trouble shooting' },
    { text: 'Search Policies' }
  ];
  icon: Icon = {
    margin: {
      top: '8px',
      left: '10px'
    },
    svgIcon: 'policy-health'
  };
  veniceTotalPolicies: number;
  veniceTotalViolatedPolicies: number;
  veniceTotalOkPolicies: number;
  topViolatedPolicies: number;
  topMostAppliedPolicies: number;
  data: any;
  options: any;

  constructor() {
  }

  ngOnInit() {
    this.id = 'policyhealth-' + Utility.s4();
    this.generateData();
    this._background_img = this.setBackgroundImg();
  }

  menuselect(obj) {
    console.log('policyhealth menuselect()', obj);
  }

  generateData() {
    const totalPolicies = Utility.getRandomInt(2000, 5000);
    const goodPercent = Utility.getRandomInt(80, 95);
    const badPercent = 100 - goodPercent;
    const goodPolicies = Math.floor(totalPolicies * goodPercent / 100);
    const badPolicies = Math.floor(totalPolicies * badPercent / 100);

    this.veniceTotalPolicies = totalPolicies;
    this.veniceTotalViolatedPolicies = badPolicies;
    this.veniceTotalOkPolicies = goodPolicies;
    this.topViolatedPolicies = MockDataUtil.getTopPolicyHealthRecords(10, 50, 100, 'policy-id ', 'policy-name ');
    this.topMostAppliedPolicies = MockDataUtil.getTopPolicyHealthRecords(10, 500, 1000, 'policy-id ', 'policy-name ');

    this.data = {
      labels: ['Healthy', 'Violated'],
      datasets: [
        {
          data: [goodPercent, badPercent],
          backgroundColor: [
            '#a3cbf6',
            '#d66340'
          ],
          /* hoverBackgroundColor: [
            '#a3cbf6',
            '#36A2EB'
          ] */
        }
      ]
    };
    this.options = {
      tooltips: {
        enabled: false
      },
      title: {
        display: false
      },
      legend: {
        display: false
      },
      circumference: 2 * Math.PI,
      rotation: (1.0 + badPercent / 100) * Math.PI, // work

      plugins: {
        datalabels: {
          backgroundColor: function(context) {
            return context.dataset.backgroundColor;
          },
          borderColor: 'white',
          borderRadius: 25,
          borderWidth: 2,
          color: 'white',
          display: function(context) {
            // print bad % only
            return context.dataIndex > 0;
          },
          font: {
            weight: 'bold',
            family: 'Fira San'
          },
          formatter: Math.round
        }
      },
      animation: {
        onComplete: function() {
          //
          // see. dashboard.component.html <ng-template #dashboardPolicyHealth . There is a <canvas id="policy_health_text"
          // we employ the chart.js onComplete() to draw "8%" to the center of donut chart
          //
          const total = badPercent + '%';
          const $ = Utility.getJQuery();
          const element = $('#policy_health_text').get(0);
          if (element) {
            const textCtx = element.getContext('2d');
            textCtx.textAlign = 'center';
            textCtx.textBaseline = 'middle';
            textCtx.font = '36px Fira San';
            textCtx.fillText(total, 190, 95);
          }
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
  }

  ngOnChanges() {
  }

  itemClick($event) {
    const obj = {
      id: this.id,
      event: $event
    };
    console.log('id', obj);
  }

  onTopViolatePolicyClick($event, policy) {
    const obj = {
      id: this.id,
      event: $event,
      policy: policy,
      selectType: 'violation'
    };
    console.log('id', obj);
  }

  onTopAppliedPolicyClick($event, policy) {
    const obj = {
      id: this.id,
      event: $event,
      policy: policy,
      selectType: 'applied'
    };
    console.log('id', obj);
  }

}
