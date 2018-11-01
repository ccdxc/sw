import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { HerocardComponent } from './herocard.component';
import { WidgetsModule } from 'web-app-framework';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { DsbdwidgetheaderComponent } from '@app/components/shared/dsbdwidgetheader/dsbdwidgetheader.component';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SpinnerComponent } from '../spinner/spinner.component';
import { RouterTestingModule } from '@angular/router/testing';


describe('HerocardComponent', () => {
  let component: HerocardComponent;
  let fixture: ComponentFixture<HerocardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        HerocardComponent,
        DsbdwidgetheaderComponent,
        PrettyDatePipe,
        SpinnerComponent,
      ],
      imports: [
        WidgetsModule,
        MaterialdesignModule,
        NoopAnimationsModule,
        RouterTestingModule
      ],
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(HerocardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
