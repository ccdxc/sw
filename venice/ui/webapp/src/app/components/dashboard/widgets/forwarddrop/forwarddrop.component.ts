import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { KeyOperatorValueKeyword, TimeRange } from '@app/components/shared/timerange/utility';
import { GraphConfig } from '@app/models/frontend/shared/userpreference.interface';

@Component({
    selector: 'app-forwarddrop',
    templateUrl: './forwarddrop.component.html',
    styleUrls: ['./forwarddrop.component.scss'],
    animations: [Animations],
    encapsulation: ViewEncapsulation.None
})
export class ForwardDropComponent implements OnInit {
    selectedTimeRange: TimeRange;
    forwardDropChartConfig: GraphConfig;

    ngOnInit() {
        this.selectedTimeRange = new TimeRange(new KeyOperatorValueKeyword.instance('now', '-', 1, 'h'), new KeyOperatorValueKeyword.instance('now', '', 0, ''));
        this.forwardDropChartConfig = {
            id: 'forwarded-dropped-chart',
            graphTransforms: {
                transforms: {
                    GraphTitle: { title: 'Dropped Counts (past 1 hour)' },
                },
            },
            dataTransforms: [{
                transforms: {
                    ColorTransform: {
                        colors: {
                            'SessionSummaryMetrics-NumDropSessions': '#ff9cee',
                        }
                    },
                },
                measurement: 'SessionSummaryMetrics',
                fields: ['NumDropSessions']
            }]
        };
    }
}
