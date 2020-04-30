import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { InlineButtonsComponent } from './inlinebuttons.component';
import { PrimengModule } from '@app/lib/primeng.module';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { HttpClientTestingModule } from '@angular/common/http/testing';

describe('InlineButtonsComponent', () => {
  let component: InlineButtonsComponent;
  let fixture: ComponentFixture<InlineButtonsComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [InlineButtonsComponent],
      imports: [
        RouterTestingModule,
        HttpClientTestingModule,
        PrimengModule,
        FormsModule,
        MaterialdesignModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
      ],
      providers: []
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(InlineButtonsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
