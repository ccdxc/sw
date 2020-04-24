import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { HerocardComponent } from './herocard.component';
import { WidgetsModule } from 'web-app-framework';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { DsbdwidgetheaderComponent } from '@app/components/shared/dsbdwidgetheader/dsbdwidgetheader.component';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SpinnerComponent } from '../spinner/spinner.component';
import { By } from '@angular/platform-browser';
import { Router } from '@angular/router';
import { CardStates, StatArrowDirection, BasecardComponent } from '../basecard/basecard.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { ControllerService } from '@app/services/controller.service';
import { NgModuleFactoryLoader } from '@angular/core';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service.ts';
import { ConfirmationService } from 'primeng/primeng';
import { AuthService } from '@app/services/auth.service';
import { HttpClient, HttpHandler } from '@angular/common/http';
import { LinegraphComponent } from '../linegraph/linegraph.component';
import { FlexLayoutModule } from '@angular/flex-layout';

const mockRouter = {
  navigateByUrl: jasmine.createSpy('navigateByUrl')
};

describe('HerocardComponent', () => {
  const stats = ['first', 'second', 'third'];
  let component: HerocardComponent;
  let fixture: ComponentFixture<HerocardComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [
        HerocardComponent,
        DsbdwidgetheaderComponent,
        PrettyDatePipe,
        SpinnerComponent,
        BasecardComponent,
        LinegraphComponent,
      ],
      imports: [
        WidgetsModule,
        MaterialdesignModule,
        NoopAnimationsModule,
        FlexLayoutModule,
      ],
      providers: [
        { provide: Router, useValue: mockRouter },
        UIConfigsService,
        LicenseService,
        ControllerService,
        NgModuleFactoryLoader,
        LogService,
        LogPublishersService,
        MessageService,
        ConfirmationService,
        AuthService,
        HttpClient,
        HttpHandler,
      ]
    }); });

  beforeEach(() => {
    fixture = TestBed.createComponent(HerocardComponent);
    component = fixture.componentInstance;
  });

  it('should allow stats to be clickable and have tooltip', () => {
    fixture.detectChanges();
    // Check stat clicking
    component.cardState = CardStates.READY;
    component.firstStat = {
      numericValue : 0,
      value: 'firstStatVal',
      description: 'firstStatDesc',
      url: 'firstStatUrl',
      tooltip: 'firstStatTooltip'
    };
    component.secondStat = {
      numericValue : 0,
      value: 'secondStatVal',
      description: 'secondStatDesc',
      url: 'secondStatUrl',
      tooltip: 'secondStatTooltip'
    };
    component.thirdStat = {
      numericValue : 0,
      value: 'thirdStatVal',
      description: 'thirdStatDesc',
      url: 'thirdStatUrl',
      tooltip: 'thirdStatTooltip'
    };
    fixture.detectChanges();
    stats.forEach((stat) => {
      const statVal = fixture.debugElement.query(By.css('.herocard-' + stat + '-stat-value'));
      // test routing
      statVal.nativeElement.click();
      fixture.detectChanges();
      expect(mockRouter.navigateByUrl).toHaveBeenCalledWith(stat + 'StatUrl');
      // test tooltip
      statVal.nativeElement.dispatchEvent(new MouseEvent('mouseover', {
        view: window,
        bubbles: true,
        cancelable: true
      }));
      fixture.whenRenderingDone().then(() => {
        const tooltip = fixture.debugElement.query(By.css('.mat-tooltip'));
        expect(tooltip).toBeNull();
      });
    });
  });

  it('should hide parts of the display', () => {
    component.title = 'testCard';
    component.lineData = {
      title: '',
      hideTitle: true,
      data: [],
      statColor: 'red',
      gradientStart: 'white',
      gradientStop: 'white',
      graphId: 'test',
      defaultValue: 0,
      defaultDescription: '',
      hoverDescription: '',
      isPercentage: true,
      scaleMin: 0,
    },
    // Title should always be shown
    fixture.detectChanges();
    const title = fixture.debugElement.query(By.css('.dsbd-widgetheader-title'));
    expect(title.nativeElement.textContent).toContain('testCard');

    // loading symbol should be visible
    let spinner = fixture.debugElement.query(By.css('app-spinner'));
    expect(spinner).toBeTruthy();
    component.cardState = CardStates.READY;
    fixture.detectChanges();
    spinner = fixture.debugElement.query(By.css('app-spinner'));
    expect(spinner).toBeNull();

    // Value and desc shouldn't be showing since the values are all blank
    expect(component).toBeTruthy();
    stats.forEach((stat) => {
      const statVal = fixture.debugElement.query(By.css('.herocard-' + stat + '-stat-value'));
      const statDesc = fixture.debugElement.query(By.css('.herocard-' + stat + '-stat-description'));
      expect(statVal).toBeNull();
      expect(statDesc).toBeNull();
    });

    // We check setting a value for each stat separately
    stats.forEach((stat) => {
      component[stat + 'Stat'] = {
        value: 'statValue',
        description: 'statDesc',
      };
      fixture.detectChanges();
      stats.forEach((s) => {
        const statVal = fixture.debugElement.query(By.css('.herocard-' + s + '-stat-value'));
        const statDesc = fixture.debugElement.query(By.css('.herocard-' + s + '-stat-description'));
        if (stat === s) {
          expect(statVal.nativeElement.textContent).toContain('statValue');
          expect(statDesc.nativeElement.textContent).toContain('statDesc');
        } else {
          expect(statVal).toBeNull();
          expect(statDesc).toBeNull();
        }
      });
      component[stat + 'Stat'] = {
        value: null,
        description: 'statDesc',
      };
      fixture.detectChanges();
    });

    // stat arrow should be hidden by default
    let arrows = fixture.debugElement.queryAll(By.css('.herocard-foreground-content mat-icon'));
    expect(arrows.length).toBe(0);

    component.arrowDirection = StatArrowDirection.UP;
    fixture.detectChanges();
    arrows = fixture.debugElement.queryAll(By.css('.herocard-foreground-content mat-icon'));
    // Arrow is still hidden since first stat's value is null
    expect(arrows.length).toBe(0);

    component.firstStat = {
      numericValue : 0,
      value: 'statValue',
      description: 'statDesc',
    };
    fixture.detectChanges();
    arrows = fixture.debugElement.queryAll(By.css('.herocard-foreground-content mat-icon'));
    expect(arrows.length).toBe(1);
    expect(arrows[0].nativeElement.textContent).toBe('arrow_upward');

    component.arrowDirection = StatArrowDirection.DOWN;
    fixture.detectChanges();
    arrows = fixture.debugElement.queryAll(By.css('.herocard-foreground-content mat-icon'));
    expect(arrows.length).toBe(1);
    expect(arrows[0].nativeElement.textContent).toBe('arrow_downward');


    // Graph should be hidden if there is less than one point
    let graph = fixture.debugElement.query(By.css('app-linegraph'));
    expect(graph).toBeNull();

    // Updating to add one point
    component.lineData.data = [{t: 0, y: 0}];
    component.ngOnChanges(null);
    fixture.detectChanges();
    graph = fixture.debugElement.query(By.css('app-linegraph'));
    expect(graph).toBeNull();

    // More than one point, graph should show
    component.lineData.data = [{t: 0, y: 0}, {t: 1, y: 1}];
    component.ngOnChanges(null);
    fixture.detectChanges();
    graph = fixture.debugElement.query(By.css('app-linegraph'));
    expect(graph).toBeTruthy();

    stats.forEach((stat, index) => {
      component[stat + 'Stat'] = {
        value: 'statValue',
        description: 'statDesc',
      };
    });
    component.cardState = CardStates.LOADING;

    fixture.detectChanges();

    graph = fixture.debugElement.query(By.css('app-linegraph'));
    expect(graph).toBeNull();
    stats.forEach((stat) => {
      const statVal = fixture.debugElement.query(By.css('.herocard-' + stat + '-stat-value'));
      const statDesc = fixture.debugElement.query(By.css('.herocard-' + stat + '-stat-description'));
      expect(statVal).toBeNull();
      expect(statDesc).toBeNull();
    });
    arrows = fixture.debugElement.queryAll(By.css('.herocard-foreground-content mat-icon'));
    expect(arrows.length).toBe(0);

    spinner = fixture.debugElement.query(By.css('app-spinner'));
    expect(spinner).toBeTruthy();
  });

});
