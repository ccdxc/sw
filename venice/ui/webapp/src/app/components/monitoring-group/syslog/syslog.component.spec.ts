import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { SyslogComponent } from './syslog.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { PrimengModule } from '@app/lib/primeng.module';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';

describe('SyslogComponent', () => {
  let component: SyslogComponent;
  let fixture: ComponentFixture<SyslogComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [SyslogComponent],
      imports: [
        MaterialdesignModule,
        FormsModule,
        ReactiveFormsModule,
        PrimengModule,
        NoopAnimationsModule
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SyslogComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
