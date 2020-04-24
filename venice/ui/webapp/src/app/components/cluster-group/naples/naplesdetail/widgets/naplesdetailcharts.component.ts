import { Component, OnInit, ViewEncapsulation, Input } from '@angular/core';
import { GraphConfig } from '@app/models/frontend/shared/userpreference.interface';
import { TimeRange, KeyOperatorValueKeyword } from '@app/components/shared/timerange/utility';

@Component({
  selector: 'app-naplesdetail-charts',
  templateUrl: 'naplesdetailcharts.component.html',
  styleUrls: ['naplesdetailcharts.component.scss'],
  encapsulation: ViewEncapsulation.None
})

export class NaplesdetailchartsComponent implements OnInit {
  @Input() selectedId: string;

  selectedTimeRange: TimeRange;
  graphConfigs: GraphConfig[] = [];

  constructor() { }

  ngOnInit() {
    this.selectedTimeRange = new TimeRange(new KeyOperatorValueKeyword.instance('now', '-', 24, 'h'), new KeyOperatorValueKeyword.instance('now', '', 0, ''));
    this.graphConfigs = [{
      id: 'dsc-temperature-chart',
      graphTransforms: {
        transforms: { GraphTitle: { title: 'ASIC Temperature' } }
      },
      dataTransforms: [{
        transforms: {
          ColorTransform: {
            colors: {
              'AsicTemperatureMetrics-LocalTemperature': '#97b8df',
              'AsicTemperatureMetrics-DieTemperature': '#61b3a0',
              'AsicTemperatureMetrics-HbmTemperature': '#ff9cee'
            }
          },
          FieldSelector: {
            selectedValues: [{
              keyFormControl: 'reporterID',
              operatorFormControl: 'in',
              valueFormControl: [this.selectedId]
            }]
          }
        },
        measurement: 'AsicTemperatureMetrics',
        fields: ['LocalTemperature', 'DieTemperature', 'HbmTemperature']
      }]
    },
    {
      id: 'dsc-power-chart',
      graphTransforms: {
        transforms: { GraphTitle: { title: 'ASIC Power' } }
      },
      dataTransforms: [{
        transforms: {
          ColorTransform: {
            colors: {
              'AsicPowerMetrics-Pin': '#97b8df',
              'AsicPowerMetrics-Pout1': '#61b3a0',
              'AsicPowerMetrics-Pout2': '#ff9cee'
            }
          },
          FieldSelector: {
            selectedValues: [{
              keyFormControl: 'reporterID',
              operatorFormControl: 'in',
              valueFormControl: [this.selectedId]
            }]
          }
        },
        measurement: 'AsicPowerMetrics',
        fields: ['Pin', 'Pout1', 'Pout2']
      }]
    }];
  }

}

