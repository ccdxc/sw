import { Component, Input, OnInit, ViewEncapsulation } from '@angular/core';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { Animations } from '@app/animations';
import { OnChanges } from '@angular/core/src/metadata/lifecycle_hooks';
import { Router } from '@angular/router';
import { StatArrowDirection, CardStates } from '../basecard/basecard.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';

export interface Stat {
  value: any;
  description: string;
  tooltip?: string;
  url?: string;
}

interface Data {
  x: Array<any>;
  y: Array<any>;
}



export interface HeroCardOptions {
  title: string;
  firstStat: Stat;
  secondStat: Stat;
  thirdStat: Stat;
  backgroundIcon: Icon;
  themeColor?: string;
  icon: Icon;
  lastUpdateTime?: string;
  data?: Data;
  arrowDirection?: StatArrowDirection;
  timeRange?: string;
  cardState?: CardStates;
}

@Component({
  selector: 'app-herocard',
  templateUrl: './herocard.component.html',
  styleUrls: ['./herocard.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class HerocardComponent implements OnInit, OnChanges {
  hasHover: boolean = false;
  cardStates = CardStates;

  @Input() title: string;
  @Input() firstStat: Stat;
  @Input() secondStat: Stat;
  @Input() thirdStat: Stat;
  @Input() themeColor: string;
  @Input() statColor: string = '';
  @Input() backgroundIcon: Icon;
  @Input() icon: Icon;
  @Input() lastUpdateTime;
  @Input() data: Data;
  @Input() arrowDirection: StatArrowDirection = StatArrowDirection.HIDDEN;
  @Input() timeRange: string;
  // When set to true, card contents will fade into view
  @Input() cardState: CardStates = CardStates.LOADING;

  showGraph: boolean = false;
  layout = {
    showlegend: false,
    paper_bgcolor: 'rgba(0,0,0,0)',
    plot_bgcolor: 'rgba(0,0,0,0)',
    autosize: true,
    margin: {
      t: 0,
      l: 0,
      r: 0,
      b: 0
    },
    width: 365,
    height: 40,
    yaxis: {
      autorange: false,
      range: [0, 100],
      showgrid: false,
      zeroline: false,
      showline: false,
      autotick: true,
      ticks: '',
      showticklabels: false
    },
    xaxis: {
      autorange: true,
      showgrid: false,
      zeroline: false,
      showline: false,
      autotick: true,
      ticks: '',
      showticklabels: false
    }
  };

  options = { displayModeBar: false, staticPlot: true };

  dataset = [];

  thresholds: any = {};
  conditionColors: any = {};

  constructor(private router: Router, protected uiconfigsService: UIConfigsService) { }

  ngOnChanges(changes) {
    if (this.cardState === CardStates.READY) {
      this.setupDataset();
    }
    if ( Object.keys(this.thresholds).length > 0 && Object.keys(this.conditionColors).length > 0) {
      this.updateStatColor();
    }
  }

  ngOnInit() {
    if (this.cardState === CardStates.READY) {
      this.setupDataset();
    }

    if (this.statColor === '') {
      this.statColor = '#77a746';
    }

    if (this.uiconfigsService.configFile) {
      // Percentage thresholds for "healthy", "warning" and "critical" come from config.json
      this.thresholds = this.uiconfigsService.configFile['condition-thresholds'];
      this.conditionColors = this.uiconfigsService.configFile['condition-colors'];
    }
  }

  getRGBString(r: number, g: number, b: number) {
    return 'rgb(' + r.toString() + ',' + g.toString() + ',' + b.toString() + ')';
  }

  updateStatColor() {
    if (this.thresholds) {
      let color, value;
      if ( typeof this.firstStat.value === 'string') {
        value = parseInt(this.firstStat.value.substring(0, this.firstStat.value.length - 1), 10);
        color = this.getConditionColor(value);
      } else if (typeof this.firstStat.value === 'number') {
        value = this.firstStat.value;
        color = this.getConditionColor(value);
      }

      if (!!color) {
        this.statColor = this.getRGBString(color['r'], color['g'], color['b']);
      }
    }
  }

  // Based on value we decide which category the color should lie in.
  // It either lies between "healthy" to "warning" or "warning" to "critical". (beyond the critical threshold the colors remains red)
  // The r, g, b values change linerally wrt stat value.
  // Condition Thresholds and Condition Colors are picked up from config.json
  getConditionColor(value) {
    let minVal, maxVal, minColor, maxColor;
    if (value <= this.thresholds.healthy ) {
      return this.conditionColors.healthy;
    } else if (value > this.thresholds.healthy && value <= this.thresholds.warning) {
      minVal = 0;
      maxVal = this.thresholds.warning;
      minColor = this.conditionColors.healthy;
      maxColor = this.conditionColors.warning;
    } else if (value > this.thresholds.warning && value <= this.thresholds.critical) {
      minVal = this.thresholds.warning;
      maxVal = this.thresholds.critical;
      minColor = this.conditionColors.warning;
      maxColor = this.conditionColors.critical;
    } else {
      return this.conditionColors.critical;
    }

    const ratio = (value - minVal) / (maxVal - minVal);

    const color = {};
    for (const key of Object.keys(minColor)) {
      color[key] = Math.floor( (maxColor[key] - minColor[key]) * ratio + minColor[key]);
    }

    return color;
  }

  private setupDataset() {
    // If we only have one data point, we don't show the graph
    if (this.data.x.length <= 1) {
      this.showGraph = false;
      return;
    }
    this.dataset = [
      // passed in graph data
      {
        x: this.data.x,
        y: this.data.y,
        type: 'scatter',
        mode: 'lines',
        line: {
          shape: 'linear',
          color: this.themeColor,
          width: 1
        }
      },
      // second trace to add marker on last point
      {
        x: [this.data.x[this.data.x.length - 1]],
        y: [this.data.y[this.data.y.length - 1]],
        type: 'scatter',
        mode: 'marker',
        line: {
          color: this.themeColor,
        }
      }
    ];
    this.showGraph = true;
  }

  firstStatClick() {
    if (this.firstStat.value != null && this.firstStat.url != null) {
      this.router.navigateByUrl(this.firstStat.url);
    }
  }

  secondStatClick() {
    if (this.secondStat.value != null && this.secondStat.url != null) {
      this.router.navigateByUrl(this.secondStat.url);
    }
  }

  thirdStatClick() {
    if (this.thirdStat.value != null && this.thirdStat.url != null) {
      this.router.navigateByUrl(this.thirdStat.url);
    }
  }

}
