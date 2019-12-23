import { Component, Input, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { Animations } from '@app/animations';
import { OnChanges, SimpleChanges } from '@angular/core/src/metadata/lifecycle_hooks';
import { Router } from '@angular/router';
import { StatArrowDirection, CardStates } from '../basecard/basecard.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LinegraphComponent, LineGraphStat, GraphPadding } from '../linegraph/linegraph.component';
import { Utility } from '@app/common/Utility';

export interface Stat {
  value: any;
  numericValue: number;
  description: string;
  tooltip?: string;
  url?: string;
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
  lineData?: LineGraphStat;
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
  @ViewChild('lineGraph') lineGraphComponent: LinegraphComponent;

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
  @Input() lastUpdateTime: string;
  @Input() lineData: LineGraphStat;
  @Input() arrowDirection: StatArrowDirection = StatArrowDirection.HIDDEN;
  @Input() timeRange: string;
  // When set to true, card contents will fade into view
  @Input() cardState: CardStates = CardStates.LOADING;

  showGraph: boolean = false;
  thresholds: any = {};
  conditionColors: any = {};

  linegraphStats: LineGraphStat[] = [];

  graphPadding: GraphPadding = {
    top: 5,
    right: 15,
    bottom: 10,
    left: 15,
  };

  prevLineGraphDataLength: number;

  constructor(private router: Router, protected uiconfigsService: UIConfigsService) { }

  ngOnChanges(changes: SimpleChanges) {
    if (this.cardState === CardStates.READY) {
      this.setupDataset();
    }

    if (changes && changes.lastUpdateTime) {
      // whenever we refresh last updated time, make sure chart is updated with most recent datapoints
      if (this.lineGraphComponent && this.lineData.data.length !== this.prevLineGraphDataLength) {
        this.lineGraphComponent.setupCharts();
      }
      this.prevLineGraphDataLength = this.lineData.data.length;
    }
  }

  ngOnInit() {
    this.linegraphStats = [this.lineData];
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

  updateStatColor(stat) {
    if (Object.keys(this.thresholds).length > 0 && Object.keys(this.conditionColors).length > 0 && stat.numericValue !== null) {
      let color, value;
      if ( typeof stat.numericValue === 'string') {
        value = parseInt(stat.numericValue.substring(0, stat.numericValue.length - 1), 10);
        color = this.getConditionColor(value);
      } else if (typeof stat.numericValue === 'number') {
        value = stat.numericValue;
        color = this.getConditionColor(value);
      }
      return this.getRGBString(color['r'], color['g'], color['b']);
    } else {
      return this.themeColor;
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
    if (this.lineData == null) {
      this.showGraph = false;
      return;
    }
    if (this.lineData.data.length <= 1) {
      this.showGraph = false;
      return;
    }
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

  /**
   *  compute whether add extra css class
   *  for VS-736
   */
  isValueLong(thirdStat: any): string {
    return  (thirdStat.value.length > Utility.HERO_CARD_THIRDVALUE_LENGTH) ? 'herocard-third-stat-value-long' : '';
  }

}
