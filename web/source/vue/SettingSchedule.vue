<template>
  <ElRow>
    <ElCol :offset="9" :span="12">
      <div class="well schedule">
        <div class="schedule-week">
          <ElCheckboxGroup v-model="innerValue.dayOfWeekSelect" size="small" :min="1" @input="TimeSet">
            <ElCheckboxButton v-for="day of weekDays" :label="day" :key="day">
              {{ day }}
            </ElCheckboxButton>
          </ElCheckboxGroup>
          <ElButton v-if="timeRange" class="schedule-button" type="text" size="small" circle icon="el-icon-minus" @click="$emit('remove', $event)" />
        </div>
        <div v-if="timeRange" class="schedule-week">
          <div class="schedule-time">
            <ElSwitch v-model="allDay" active-text="終日" @input="TimeSet" />
            <div v-if="!allDay" class="schedule-timerange">
              <ElTimePicker class="time-picker" v-model="innerValue.startTime" placeholder="開始時間" value-format="HH:mm" format="HH:mm" @change="TimeSet" />
              -
              <ElTimePicker class="time-picker" v-model="innerValue.endTime" placeholder="終了時間" value-format="HH:mm" format="HH:mm" @change="TimeSet" />
            </div>
          </div>
          <ElButton v-if="timeRange" class="schedule-button" type="text" size="small" circle icon="el-icon-plus" @click="$emit('add', $event)" />
        </div>
        <div v-else-if="timelapse">
          <div class="schedule-time">
            <ElTimePicker class="time-picker" v-model="innerValue.startTime" placeholder="開始時間" value-format="HH:mm" format="HH:mm" @change="TimeSet" />
            <span class="strings"> 〜 {{ timelapseEndTime }}まで</span>
          </div>
          <div class="schedule-time">
            <span class="strings">周期</span>
            <ElInputNumber class="timelapse-number" v-model="innerValue.interval" @change="TimeSet" :min="1" :controls="false" :step-strictly="true" size="mini" />
            <span class="strings">秒</span>
            <ElInputNumber class="timelapse-number" v-model="innerValue.count" @change="TimeSet" :min="1" :controls="false" :step-strictly="true" size="mini" />
            <span class="strings">回</span>
          </div>
        </div>
        <div v-else class="schedule-week schedule-time">
          <ElTimePicker class="time-picker" v-model="innerValue.startTime" placeholder="設定時間" value-format="HH:mm" format="HH:mm" @change="TimeSet" />
        </div>
      </div>
    </ElCol>
  </ElRow>
</template>

<script>
  import { Switch, CheckboxGroup, CheckboxButton, TimePicker, InputNumber } from 'element-ui';
  import 'element-ui/lib/theme-chalk/switch.css';
  import 'element-ui/lib/theme-chalk/checkbox.css';
  import 'element-ui/lib/theme-chalk/time-picker.css';
  import 'element-ui/lib/theme-chalk/input-number.css';

  export default {
    components: {
      ElSwitch: Switch,
      ElCheckboxGroup: CheckboxGroup,
      ElCheckboxButton: CheckboxButton,
      ElTimePicker: TimePicker,
      ElInputNumber: InputNumber,
    },
    props: {
      timeRange: {
        type: Boolean,
        default: false,
      },
      timelapse: {
        type: Boolean,
        default: false,
      },
      value: {
        type: Object,
        required: true,
      },
    },
    data() {
      return {
        innerValue: this.value,
        weekDays: ['月','火', '水', '木', '金', '土', '日'],
        allDay: (this.value.startTime === '00:00') && (this.value.endTime === '23:59'),
      };
    },
    computed: {
      timelapseEndTime() {
        const startTime = (this.innerValue.startTime || '').split(':');
        let minutes = ((startTime.length === 2) ? parseInt(startTime[0]) * 60 + parseInt(startTime[1]) : 0) + (this.value.count * this.value.interval) / 60;
        let str = '';
        if(minutes >= 48 * 60) {
          str += `${ Math.floor(minutes / 24 / 60) }日後の`;
        } else if(minutes >= 24 * 60) {
          str += '翌日';
        }
        str += Math.floor((minutes % 1440) / 60).toString().padStart(2, '0') + ':';
        str += Math.floor(minutes % 60).toString().padStart(2, '0');
        return str;
      },
    },
    watch: {
      value(v) {
        this.innerValue = { ...v };
      },
    },
    methods: {
      TimeSet() {
        if(this.timeRange && this.allDay) {
          this.innerValue.startTime = '00:00';
          this.innerValue.endTime = '23:59';
        }
        this.$emit('input', this.innerValue);
      },
    },
  };
</script>

<style scoped>
  .schedule {
    display: inline-flex;
    flex-direction: column;
    width: 95%;
  }
  .schedule-week {
    display: inline-flex;
    justify-content: space-between;
  }
  .schedule-time {
    display: inline-flex;
    margin:10px 5px 5px 5px;
  }
  .schedule-timerange {
    margin:0px 0px 0px 10px;
  }
  .schedule-button {
    padding: 0;
    text-alin:right;
    float:right;
  }
  .time-picker {
    width: 110px;
  }
  .timelapse-number {
    margin:0px 0px 0px 10px;
  }
  .strings {
    margin:0px 5px 0px 5px;
    font-size: 1.1em;
  }
</style>
<style>
  .el-checkbox-button.is-disabled.is-checked .el-checkbox-button__inner {
    color: #C0C4CC;
    cursor: not-allowed;
    background-image: none;
    background-color: #409EFF;
    border-color: #409EFF;
    -webkit-box-shadow: none;
    box-shadow: none;
  }
</style>
