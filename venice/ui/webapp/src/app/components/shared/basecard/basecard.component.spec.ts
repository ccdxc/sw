import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { BasecardComponent } from './basecard.component';
import { WidgetsModule } from 'web-app-framework';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { DsbdwidgetheaderComponent } from '@app/components/shared/dsbdwidgetheader/dsbdwidgetheader.component';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SpinnerComponent } from '../spinner/spinner.component';
import { Router } from '@angular/router';

const mockRouter = {
  navigateByUrl: jasmine.createSpy('navigateByUrl')
};

describe('HerocardComponent', () => {
  const stats = ['first', 'second', 'third'];
  let component: BasecardComponent;
  let fixture: ComponentFixture<BasecardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        BasecardComponent,
        DsbdwidgetheaderComponent,
        PrettyDatePipe,
        SpinnerComponent,
      ],
      imports: [
        WidgetsModule,
        MaterialdesignModule,
        NoopAnimationsModule,
      ],
      providers: [
        { provide: Router, useValue: mockRouter },
      ]
    }).compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(BasecardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
