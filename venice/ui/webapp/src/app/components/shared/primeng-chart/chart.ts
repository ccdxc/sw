import { Component, ElementRef, AfterViewInit, OnDestroy, Input, Output, EventEmitter } from '@angular/core';
import { Chart } from 'chart.js';

/**
 * This is copied from the primeng library, but the chart import is changed.
 */

@Component({
    selector: 'app-chart',
    template: `
        <div style="position:relative" [style.width]="responsive && !width ? null : width" [style.height]="responsive && !height ? null : height">
            <canvas [attr.width]="responsive && !width ? null : width" [attr.height]="responsive && !height ? null : height" (click)="onCanvasClick($event)"></canvas>
        </div>
    `
})
export class UIChartComponent implements AfterViewInit, OnDestroy {

    @Input() type: string;

    @Input() options: any = {};

    @Input() plugins: any[] = [];

    @Input() width: string;

    @Input() height: string;

    @Input() responsive: boolean = true;

    @Output() onDataSelect: EventEmitter<any> = new EventEmitter(); // tslint:disable-line

    initialized: boolean;

    _data: any;

    chart: any;

    constructor(public el: ElementRef) {}

    @Input() get data(): any {
        return this._data;
    }

    set data(val: any) {
        this._data = val;
        this.reinit();
    }

    ngAfterViewInit() {
        this.initChart();
        this.initialized = true;
    }

    onCanvasClick(event) {
        if (this.chart) {
            const element = this.chart.getElementAtEvent(event);
            const dataset = this.chart.getDatasetAtEvent(event);
            if (element && element[0] && dataset) {
                this.onDataSelect.emit({originalEvent: event, element: element[0], dataset: dataset});
            }
        }
    }

    initChart() {
        const opts = this.options || {};
        opts.responsive = this.responsive;

        // allows chart to resize in responsive mode
        if (opts.responsive && (this.height || this.width)) {
            opts.maintainAspectRatio = false;
        }

        this.chart = new Chart(this.el.nativeElement.children[0].children[0], {
            type: this.type,
            data: this.data,
            options: this.options,
            plugins: this.plugins
        });
    }

    getCanvas() {
        return this.el.nativeElement.children[0].children[0];
    }

    getBase64Image() {
        return this.chart.toBase64Image();
    }

    generateLegend() {
        if (this.chart) {
            return this.chart.generateLegend();
        }
    }

    refresh() {
        if (this.chart) {
            this.chart.update();
        }
    }

    reinit() {
        if (this.chart) {
            this.chart.destroy();
            this.initChart();
        }
    }

    ngOnDestroy() {
        if (this.chart) {
            this.chart.destroy();
            this.initialized = false;
            this.chart = null;
        }
    }
}
